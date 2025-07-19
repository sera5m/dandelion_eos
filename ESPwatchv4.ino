//do not touch
#include "Wiring.h" //my hardware definitions
//esp32-s3 hardware
#include "USB.h"
#include "USBCDC.h"
#include <Wire.h>
#include <esp32-hal-spi.h>
#include "apps.ino"//GOSHIES I WONDER WHAT COULD BE IN HERE
#include "NFC.h"
#include "driver/timer.h"
#include <sstream>
#define CLAMP(val, min, max) (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val))

#define DO_ONCE(name)       \
    static bool _did_##name = false; \
    if (!_did_##name)       \
        for (_did_##name = true; _did_##name; _did_##name = false)
#define RESET_DO_ONCE(name) (_did_##name = false)

#define DBG_PRINTLN(x)  do { \
    tft.setCursor(0, _dbg_ypos); \
    tft.setTextColor(WHITE); \
    tft.setTextSize(1); \
    tft.println(x); \
    _dbg_ypos += 10; \
} while(0)

int _dbg_ypos = 0;  // screen Y cursor
uint32_t clocki32cache=0; 
//stupid fucking debugs to make this work right
typedef enum{WM_MAIN, WM_STOPWATCH,WM_ALARMS,WM_TIMER,WM_NTP_SYNCH, WM_SET_TIME,WM_SET_TIMEZONE,WM_APPMENU,WM_COUNT}WatchMode;
typedef enum{HMM_BIOMONITOR, //Current fuckshit like that beep boop beeip in hopital
HMM_DAYHISTORY,    //a bar graph over the past x days.
   HMM_HISTORY, //this month/historical trends. on long scales of time we'll just store average hr as waking/sleeping 
   HMM_SETTINGS   //idk man what do you even config here?
}HealthmonitorMode;

//typedef enum{NFC_OFF,NFC_PLAYBACK,NFC_RECORD,}NFC_MODE;
//remember
typedef enum {
    GSLC_POWER,         // sleep modes
    GSLC_ALERTS,        // notifications, alarms
    GSLC_DISPLAY,       // screen settings
    GSLC_DATA,          // storage, sd card
    GSLC_WIRELESS,      // wifi, bt
    GSLC_EXT_HARDWARE,  // modules, sensors
    GSLC_CATEGORY_COUNT
} GlobalSettingsListCategory;
typedef enum {
    PWR_ULTRA,
    PWR_AGGRESSIVE,
    PWR_HEAVY,
    PWR_MODERATE,
    PWR_LIGHT,
    PWR_NONE
} PowerMode;

typedef enum {
    ALERT_SRC_PHONE,
    ALERT_SRC_MISC_INTERNAL,
    ALERT_SRC_CLOCK
} AlertSource;
//#include "watch_Settings.h" //configuration file for settings
const char* GlobalSettingsListCategoryNames[] = {
    "power",        // sleep modes
    "alerts",       // notifications, alarms
    "display",      // screen settings
    "data",         // storage, sd card
    "wireless",     // wifi, bt
    "ext_hardware"  // modules, sensors
};


typedef struct {
PowerMode mode;
} PowerSettings;

typedef struct {
    uint8_t intensity;  // 0-255 maybe
    int flash_light;    // bool
    AlertSource source;
} AlertsSettings;

typedef struct {
    uint8_t brightness; // 0-255
    int greyscale;      // bool
    int fast_refresh;   // bool
} DisplaySettings;

typedef struct {
    int nfc_enabled;  // bool
    int wifi_enabled; // bool
    int bt_enabled;   // bool
} WirelessSettings;

typedef struct {
    uint32_t storage_used_mb;
    uint32_t storage_total_mb;
} DataSettings;
typedef struct {
    PowerSettings power;
    AlertsSettings alerts;
    DisplaySettings display;
    DataSettings data;
    WirelessSettings wireless;
    // ext_hardware can be fleshed out later
} GlobalSettings;


const char* appNames[] = {
  "lock screen",
  "health",
  "NFC Tools",
  "Settings",
  "Gyro Info",
  "Files",
  "Radio",
  "IR Remote",
  "utilities",
  "eTools", // combination of oscilloscope and signal gen
  "rubberducky",
  "connections",
  "smart devices",
  "Diagnostics"
};

typedef enum {
  APP_LOCK_SCREEN,
  APP_HEALTH,
  APP_NFC_TOOLS,
  APP_SETTINGS,
  APP_GYRO_INFO,
  APP_FILES,
  APP_RADIO,
  APP_IR_REMOTE,
  APP_UTILITIES,
  APP_ETOOLS,
  APP_RUBBERDUCKY,
  APP_CONNECTIONS,
  APP_SMART_DEVICES,
  APP_DIAGNOSTICS,
  APP_COUNT // always handy for bounds checking
}AppName;




#include <SPI.h>
SPIClass spiBus(HSPI);


//esp32 specifiic
#include "esp_pm.h"
#include "esp_wifi.h" 
#include <WiFi.h>
#include "esp_bt.h"   //
#include "esp_sleep.h"
#include "esp_system.h" 
#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"

#include "esp_timer.h"
#include "esp_system.h"

//std 
#include <stdbool.h>
#include <cstdlib>  // For rand()
#include <ctime>  
#include <vector>
#include <stdint.h> 
#include <iomanip> 
#include <chrono>

//rots
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <mutex> 
#include "HardwareSerial.h"

//arduino compats
#include <Wire.h>
#include <time.h>
#include <stdio.h>



//hardware==============================================

#include "driver/pulse_cnt.h"
#include "esp_check.h"

// PCNT unit assignments
#define PCNT_UNIT_X PCNT_UNIT_0
#define PCNT_UNIT_Y PCNT_UNIT_1

// Encoder channel assignments
#define PCNT_CHANNEL_X PCNT_CHANNEL_0
#define PCNT_CHANNEL_Y PCNT_CHANNEL_0  // Separate unit gets its own channel 0


//heartrate
#include "MAX30105.h" //sparkfun lib
#include "heartRate.h"
#include "spo2_algorithm.h"//aw lawd
#include "mdl_heartmonitor.ino" //my bad code
MAX30105 particleSensor;//If this sensor were accurate, it would report your heart rate spiking every time i try to debug this code.
   
//imu
#include "FastIMU.h" 
#include "mdl_accelerometer.ino"
#define IMU_ADDRESS 0x68    //0x68 is the imu adress, if it needs to be fixed do that later
//#define PERFORM_CALIBRATION //Comment to disable startup calibration
MPU6500 IMU;               //Change to the name of any supported IMU! -extern so we can access this in accel module
calData calib = { 0 };  //Calibration data
AccelData imuAccel;      //Sensor data
GyroData gyroData;


//time
#include "mdl_clock.h"

//storate
#include "SDFS.ino"
#include <SD.h> //esp specific lib
#include <nvs_flash.h>
#include <nvs.h>

#include <pgmspace.h>
#include "IR_Remote.ino"

#include "inputHandler.h"

//include my own stuff
 //#include "AT_SSD1351.ino"
#include "Micro2D_A.ino"  // The library

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Adafruit_NeoPixel.h>
int16_t temp_c=69;
extern int AVG_HR;

//fuckj this were gonna put it wice
// Globals.cpp or at the top of your main .ino (outside setup and loop)


bool deviceIsAwake=true;

Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST); //note: &spiBus is required to pass main spi 
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800); //flashlight 

//init windows
WindowManager* windowManagerInstance = nullptr;

static std::shared_ptr<Window> lockscreen_clock; QueueHandle_t lockscreenQueue = nullptr;
static std::shared_ptr<Window> lockscreen_biomon;
static std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;

// timer_editor.h
typedef enum {
    EDIT_OFF,        // Normal display mode
    EDIT_RUNNING,    // Actively editing values
    EDIT_CONFIRM     // Save/cancel prompt
} EditState;

#define NUM_TIMERS 5  // Replace magic number
#define NUM_TIMER_FIELDS 2  // For HH:MM editing

 EditState timerEditState;
 uint8_t currentTimerField;  // Replaces currentTimerField
 uint8_t selectedTimerIndex;



#include <atomic>


uint16_t tcol_primary=0x0EFF;
uint16_t tcol_secondary=0x88fB;
uint16_t tcol_tertiary=0xe4ff;
uint16_t tcol_highlight=0xdbbf;
uint16_t tcol_background=0x2000;
//var references to set for the theme
 //   0x07ff, //teal
   // 0x77f9, //i can't find a good green
   // 0xe4ff,//lavender
   // 0xd7fd,//very light green highlight
   // 0x29e6//background
//list_Themes Current_Theme=mint; //set the current theme to a nice default




extern QueueHandle_t processInputQueue;
#define MAX_VISIBLE 15
char buf_applist[25*MAX_VISIBLE]; //ext for better access

bool is_watch_screen_in_menu=false;
bool isConfirming = false;


uint8_t AppMenuSelectedIndex=0; //app menu.yeah its global so we can share btwn 
uint8_t CurrentOpenApplicationIndex=0; //for self referential current code


bool stopwatchRunning = false;
unsigned long stopwatchStart = 0;


//nfc-rfid
#include <Adafruit_PN532.h>
//#include <Adafruit_Sensor.h>
Adafruit_PN532 nfc(HSPI, SPI_CS_NFC); // Create an instance of the PN532 using hardware SPI

//wireles comunication
//#include <RadioLib.h>



//add the encoders
#include "inputHandler.h"
// Add this definition exactly once:
extern encoder_state_t encoders[2];

int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;
NormieTime CurrentNormieTime; //real current time


QueueHandle_t processInputQueue; //absolutely needs to be here because freerots. hadndles proscess input    


void split_u32_to_24(uint32_t input, uint8_t *last8, uint8_t *color) {
    // Extract 24-bit color (lower 24 bits)
    color[0] = (input >> 16) & 0xFF; // Most significant byte of 24-bit color
    color[1] = (input >> 8) & 0xFF;
    color[2] = input & 0xFF;

    // Extract last 8 bits (most significant byte)
    *last8 = (input >> 24) & 0xFF;
}//split the color

void scanI2C() {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning I2C bus...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x"); DBG_PRINTLN("ok");
      Serial.println(address, HEX); DBG_PRINTLN(String(address, HEX));
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at 0x");// DBG_PRINTLN("err at");
      Serial.println(address, HEX); //DBG_PRINTLN(String(address, HEX));
    }
  }

  if (nDevices == 0){
    Serial.println("No I2C devices found\n"); DBG_PRINTLN("0I2c devices"); }
  else{
    Serial.println("I2C scan complete\n"); DBG_PRINTLN("I2C done");}

}

#include "main_app_logic.ino"

TaskHandle_t watchScreenHandle; //handle be4 use

void setup() {
  strip.begin();
  strip.show(); 
WiFi.mode(WIFI_OFF);
btStop();
//set up structs and configs


//end setup struct+cfg
    delay(148); 
    Serial.begin(115200);

  delay(100); // Let this Bitch™ boot
//SetDeviceTheme(mint); //apply and boot

    _dbg_ypos = 0; // Reset debug print position
    //set pins up
    spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    //spiBus.begin(20000000);
    //set speed be4 any com
   SPI.beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));  // Set SPI speed to 20 MHz
   //init
    screen_on();
    screen_startup();

    // DISPLAY_CONF();

    tft.fillScreen(tcol_background);
    DBG_PRINTLN("BOOT BEGIN");

Wire.begin(SDA_PIN, SCL_PIN);
//DISPLAY_CONF();//conf disp
scanI2C();

    SetupHardwareInput();
    DBG_PRINTLN("Input OK");

    DBG_PRINTLN("Checking SD");
    if (!SD.begin(SPI_CS_SD, spiBus)) {
        DBG_PRINTLN("SD FAIL 1");
        for (int i = 0; i < 3; ++i) {
            delay(500);
            if (SD.begin(SPI_CS_SD)) {
                DBG_PRINTLN("SD OK");
                break;
            }
        }
    } else {
        DBG_PRINTLN("SD OK 1");
    }

if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    DBG_PRINTLN("HRSENSORFAILURE");
  }
else{
  Serial.println("Place your index finger or wrist on the sensor with steady pressure.");
DBG_PRINTLN("hr sensor ok");
  if (enableBloodOxygen) {
    // Configure sensor for blood oxygen mode (Red + IR)
    byte ledBrightness  = 60;
    byte sampleAverage  = 4;
    byte ledMode        = 2;      // Use Red + IR LEDs
    byte sampleRate     = 100;
    int pulseWidth      = 411;
    int adcRange        = 4096;
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  } else {
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
  }

  
}


    windowManagerInstance = WindowManager::getWinManagerInstance();
    if (!windowManagerInstance) {
        DBG_PRINTLN("WinMgr FAIL");
        return;
    } else {
        DBG_PRINTLN("WinMgr OK");
    }
//SetDeviceTheme(mint);
//tft.setContrast(254);

CREATE_LOCKSCREEN_WINDOWS();//CREATE DEFAULT LOCKSCREEN SHIT
   // DBG_PRINTLN("Thermo OK");
//SetDeviceTheme(mint);//change the color pallette refs

TFillRect(0,0,128,128,tcol_background);//black screen out


windowManagerInstance->ApplyThemeAllWindows(tcol_secondary, tcol_background, tcol_primary); //with new vars



processInputQueue = xQueueCreate(8, sizeof(S_UserInput)); //set up default que
xTaskCreate(watchscreen, "WatchScreen", 4096, NULL, 1, NULL);//core 0 watch screen 

xTaskCreatePinnedToCore(
    INPUT_tick,         // Task function
    "INPUT_tick",       // Name
    2048,               // Stack size
    NULL,               // Params
    2,                  // Priority
    &watchScreenHandle, // Handle
    1                   // Core
);


//evil spagetti
//processInputQueue = lockscreenQueue;//2. tell input router to use that que
currentinputTarget = R_toProc; //3. MANUALLY alter input handling values to route to proscesses. we



    DBG_PRINTLN("SETUP DONE");
    delay(100);
    tft.fillScreen(tcol_background);   
}//end void setup



    int WatchScreenUpdateInterval=500;//declaration




void clearScreenEveryXCalls(uint16_t x) {
    static uint16_t callCount = 0;
    
    callCount++;
    
    if (callCount >= x) {
        tft.fillRect(0, 0, 128, 128, tcol_background);
        callCount = 0;
    }
}


    // Shared buffers for display
    char watchscreen_buf[120];
    char thermoStr[8];
    char hrStr[8];
    
uint8_t watchModeIndex = 0; //likely best to rename to mouse pos lol
//need these to persist

  
  //need struct for theother thing here for alarm mode
  usr_alarm_st usralmstbuf;//cache one for setters

void onVertical_input_timer_buff_setter(bool direction) {
    int8_t dir = direction ? 1 : -1;

    uint8_t color[3];
    uint8_t days_bitmask;

    // Split combined LightColor
    split_u32_to_24(usralmstbuf.LightColor, &days_bitmask, color);

    switch (currentTimerField) {
    case 0: // min 1s digit
{
    int mins = usralmstbuf.minutes;
    int ones = mins % 10;
    ones += dir;
    if (ones > 9) ones = 0;
    if (ones < 0) ones = 9;
    usralmstbuf.minutes = (mins / 10) * 10 + ones;
}
break;

case 1: // min 10s digit
{
    int mins = usralmstbuf.minutes;
    int tens = mins / 10;
    tens += dir;
    if (tens > 5) tens = 0; // max tens digit for mins
    if (tens < 0) tens = 5;
    usralmstbuf.minutes = tens * 10 + (mins % 10);
}
break;

case 2: // hour 1s digit
{
    int hrs = usralmstbuf.hours;
    int tens = hrs / 10;
    int ones = hrs % 10;
    ones += dir;

    int max_ones = (tens == 2) ? 3 : 9;

    if (ones > max_ones) ones = 0;
    if (ones < 0) ones = max_ones;

    usralmstbuf.hours = tens * 10 + ones;
}
break;

case 3: // hour 10s digit
{
    int hrs = usralmstbuf.hours;
    int tens = hrs / 10;
    int ones = hrs % 10;

    tens += dir;
    if (tens > 2) tens = 0;
    if (tens < 0) tens = 2;

    int max_ones = (tens == 2) ? 3 : 9;
    if (ones > max_ones) ones = max_ones;

    usralmstbuf.hours = tens * 10 + ones;
}
break;

case 4: // alarm action
{
    int temp_action = (int)usralmstbuf.E_AlarmAction + dir;

    if (temp_action > ALARM_ACTION_MAX) temp_action = 0;
    if (temp_action < 0) temp_action = ALARM_ACTION_MAX;

    usralmstbuf.E_AlarmAction = (alarmAction)temp_action;
}
break;

    case 5: // snooze duration
        usralmstbuf.SnoozeDur += dir;
        if (usralmstbuf.SnoozeDur > 30) usralmstbuf.SnoozeDur = 1;
        if (usralmstbuf.SnoozeDur < 1) usralmstbuf.SnoozeDur = 30;
        break;

    case 6: case 7: case 8: case 9: case 10: case 11: case 12: {
        // Days bitmask toggle: Mon–Sun
        uint8_t bit = currentTimerField - 6;
        if (bit < 7) {
            if (direction) {
                days_bitmask |= (1 << bit);  // toggle ON
            } else {
                days_bitmask &= ~(1 << bit); // toggle OFF
            }
        }
        break;
    }

    case 13: // Color R
        color[0] = (uint8_t)CLAMP(color[0] + dir, 0, 255);
        break;
    case 14: // Color G
        color[1] = (uint8_t)CLAMP(color[1] + dir, 0, 255);
        break;
    case 15: // Color B
        color[2] = (uint8_t)CLAMP(color[2] + dir, 0, 255);
        break;

    default:
        currentTimerField = 0; // fallback
        break;
    }

    // Recombine LightColor
    usralmstbuf.LightColor =
        ((uint32_t)days_bitmask << 24) |
        ((uint32_t)color[0] << 16) |
        ((uint32_t)color[1] << 8) |
        color[2];
}
std::string formatAlarm(uint8_t mousepos, bool confirmMode) {
    uint8_t days_bitmask, color24[3];
    split_u32_to_24(usralmstbuf.LightColor, &days_bitmask, color24);

    // Time display with label and bracket highlighting
    char tmp[6];
    snprintf(tmp, sizeof(tmp), "%02u:%02u", usralmstbuf.hours, usralmstbuf.minutes);
    std::string timeStr = "Time: ";
    for (uint8_t i = 0; i < 5; i++) {
        if (i == 2) { // Colon
            timeStr += tmp[i];
            continue;
        }
        int8_t posMap = -1;
        switch (mousepos) {
            case 3: posMap = 0; break; // Hour tens
            case 2: posMap = 1; break; // Hour ones
            case 1: posMap = 3; break; // Minute tens
            case 0: posMap = 4; break; // Minute ones
            default: break;
        }
        if (i == posMap) {
            timeStr += "<setcolor(0xF005)>[" + std::string(1, tmp[i]) + "]<setcolor(0x07ff)>";
        } else {
            timeStr += tmp[i];
        }
    }

    // Action string with highlight
    std::string actionStr = AlarmActionNames[usralmstbuf.E_AlarmAction];
    if (mousepos == 4) {
        actionStr = "<setcolor(0xF005)>[" + actionStr + "]<setcolor(0x07ff)>";
    }

    // Days of week with proper formatting
    std::string daysStr = "days active:<n>";
    for (uint8_t d = 0; d < 7; d++) {
        bool active = days_bitmask & (1 << d);
        std::string day = DayNames[d];
        
        if (mousepos == 6 + d) {
            // Highlighted (red)
            day = "<setcolor(0xF005)>" +day+ "<setcolor(0x07ff)>";
        } else if (active) {
            // Active (green)
            day = "<setcolor(0x07E0)>" +day+ "<setcolor(0x07ff)>";
        } else {
            // Inactive (blue)
            day = "<setcolor(0x001F)>" +day+ "<setcolor(0x07ff)>";
        }
        
        daysStr += day;
        if (d < 6) daysStr += " "; // Space between days except last
    }

    // Snooze duration with highlight
    std::string snoozeStr = std::to_string(usralmstbuf.SnoozeDur) + "min";
    if (mousepos == 5) {
        snoozeStr = "<setcolor(0xF005)>[" + snoozeStr + "]<setcolor(0x07ff)>";
    }

    // Final composition with proper newlines and formatting
    std::ostringstream out;
    out << timeStr
        << "<n><n>Alert: " << actionStr
        << "<n>" << daysStr
        << "<n><n>Snooze: " << snoozeStr
        << "<n><n>" // Extra spacing before actions
       << (confirmMode ? 
    "<setcolor(0xF005)>[SAVE]<setcolor(0x07ff)> CANCEL" : 
    "SAVE <setcolor(0xF005)>[CANCEL]<setcolor(0x07ff)>");

    return out.str();
}


//to keep the apps out of main for orginization we'll just slap those motherfuckers in here for definitions

//+++++++++++++++++++++++++++++++++++++++main screen====-------------------------------------



WindowCfg d_ls_c_cfg = { //clock
    14, 64, //xy
    100, 42, //wh
    false, false, //auto align,wraptext
    2, //text size
    true,//borderless?
    tcol_secondary, tcol_background, tcol_primary, // <-- pass addresses!. colors
    1000 //update interval ms
};

WindowCfg d_ls_b_cfg = {//heart monitor
    86, 0,
    50, 12,
    false, false,
    1,
    true,
    tcol_secondary, tcol_background, tcol_primary,
    1000
};

WindowCfg d_ls_th_cfg = {//thermometer
    8, 0,
    50, 12,
    false, false,
    1,
    false,
    tcol_secondary, tcol_background, tcol_primary,
    1000
};

void CREATE_LOCKSCREEN_WINDOWS(){
        lockscreen_clock = std::make_shared<Window>("lockscreen_clock", d_ls_c_cfg, "HH:MM:SS");
    windowManagerInstance->registerWindow(lockscreen_clock);
    DBG_PRINTLN("Clock OK");

            lockscreen_biomon = std::make_shared<Window>("lockscreen_biomon", d_ls_b_cfg, "XXXbpm");
    windowManagerInstance->registerWindow(lockscreen_biomon);
    DBG_PRINTLN("Biomon OK");

        lockscreen_thermometer = std::make_shared<Window>("lockscreen_thermometer", d_ls_th_cfg, "XXXC");
    windowManagerInstance->registerWindow(lockscreen_thermometer);

}
//i put this above everything to avoid bugs because .ino is evil. typedef enum{WM_MAIN, WM_STOPWATCH,WM_ALARMS,WM_TIMER,WM_NTP_SYNCH, WM_SET_TIME,WM_SET_TIMEZONE}WatchMode;
//app-appmenu==============================================================================
  void transitionApp(uint8_t index) {
    AppName app = (AppName)index;

    switch (app) {
        case APP_LOCK_SCREEN:
            // Do something for lock screen
            CurrentOpenApplicationIndex=APP_LOCK_SCREEN; //note! need to set this on sucess, move this later to outside just this mode. this is the current open app n should only be set on success!
            break;
        case APP_HEALTH:
            // Do something for health app
            break;
        case APP_NFC_TOOLS:
            // ...
            break;
        case APP_SETTINGS:
            // ...
            break;
        case APP_GYRO_INFO:
            // ...
            break;
        case APP_FILES:
            // ...
            break;
        case APP_RADIO:
            // ...
            break;
        case APP_IR_REMOTE:
            // ...
            break;
        case APP_UTILITIES:
            // ...
            break;
        case APP_ETOOLS:
            // ...
            break;
        case APP_RUBBERDUCKY:
            // ...
            break;
        case APP_CONNECTIONS:
            // ...
            break;
        case APP_SMART_DEVICES:
            // ...
            break;
        case APP_DIAGNOSTICS:
            // ...
            break;

        case APP_COUNT:  // Usually no action here, just for bounds
        default:
            // Handle invalid selection gracefully
            break;
    }
}
//scrolling up enters it?


void updateAppList(char *buf, size_t bufSize, const char **apps, int APP_COUNT, int AppMenuSelectedIndex) {
    buf[0] = '\0'; // clear

    int start = 0;
    if (APP_COUNT > MAX_VISIBLE) {
        start = AppMenuSelectedIndex - MAX_VISIBLE / 2;
        if (start < 0) start = 0;
        if (start + MAX_VISIBLE > APP_COUNT) start = APP_COUNT - MAX_VISIBLE;
    }

    for (int i = 0; i < MAX_VISIBLE && (start + i) < APP_COUNT; ++i) {
  int idx = start + i;

  if (idx == AppMenuSelectedIndex) {
    strncat(buf, "<setcolor(0xdbbf)>[", bufSize - strlen(buf) - 1); //i really hope the macro bullshit replaces this so this highlights right. please leave it tcol
    strncat(buf, apps[idx], bufSize - strlen(buf) - 1);
    strncat(buf, "]<setcolor(0x07ff)>", bufSize - strlen(buf) - 1);
  } else {
    strncat(buf, apps[idx], bufSize - strlen(buf) - 1);
  }

  strncat(buf, "<n>", bufSize - strlen(buf) - 1);
}

// Pad with blank lines if needed
int visibleLines = APP_COUNT < MAX_VISIBLE ? APP_COUNT : MAX_VISIBLE;
 for (int i = visibleLines; i < MAX_VISIBLE; ++i) {
  strncat(buf, "<n>", bufSize - strlen(buf) - 1);
}


    // Optionally ensure trailing newline if needed
    //strncat(buf, "<n>", bufSize - strlen(buf) - 1);
}//ene fn update applist



WatchMode currentWatchMode = WM_MAIN;
int stopwatchElapsed=0;

void WATCH_SCREEN_TRANSITION(WatchMode desiredMode){
  lockscreen_clock->updateContent("");//remove content, avoid visual only bug
switch (desiredMode){
  
//tft.fillScreen(tcol_background);//clean everything

                case WM_MAIN:
                WatchScreenUpdateInterval=500;
                //update bg? 
                //lockscreen_clock->updateContent("");//fixes bug with text overflow
                lockscreen_clock->setWinTextSize(2);
                lockscreen_clock->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                lockscreen_clock->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true); //reset to original config size reguardless of original config
                
                break;
                
                case WM_STOPWATCH:
                    WatchScreenUpdateInterval=120;//update more frequently. unfortunately, there's still an issue with latency so we'll keep 
                    //lockscreen_clock->updateContent("");//fixes bug with text overflow
                    lockscreen_clock->setWinTextSize(2);
                    lockscreen_clock->ResizeWindow(128, d_ls_c_cfg.height,false);//expand for more digits, .xyz expand by  pixels
                    lockscreen_clock->MoveWindow(d_ls_c_cfg.x-14, d_ls_c_cfg.y,true);//move 14 pixels to the left to offset more digits.
                                                        //x now has effective increased size of 28px. probably better if i did this as a new struct but who cares. 
                         //resize window and force update as of 6/25/25 have the ability to not force the screen to update, preventing graphical glitches
                 
                 break;
                

                case WM_ALARMS:
                    // TODO: Display upcoming alarms or alarm setup screen
                    WatchScreenUpdateInterval=350;
                    lockscreen_clock->setWinTextSize(1);
                    lockscreen_clock->ResizeWindow(128, 112,false);//expand for more digits, .xyz expand by  pixels
                    lockscreen_clock->MoveWindow(0,16,false); //xy

                 break;

                case WM_TIMER:
                WatchScreenUpdateInterval=350;
                lockscreen_clock->setWinTextSize(1);
               lockscreen_clock->ResizeWindow(128, 112,false);//expand for more digits, .xyz expand by  pixels
                    lockscreen_clock->MoveWindow(0,16,false);
                  break;

                case WM_NTP_SYNCH:
                WatchScreenUpdateInterval=500;
                lockscreen_clock->setWinTextSize(2);
                lockscreen_clock->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                lockscreen_clock->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,false);
                 lockscreen_clock->updateContent("time synch, wifi");

                  break;

                case WM_SET_TIME:
                WatchScreenUpdateInterval=500;
                lockscreen_clock->setWinTextSize(2);
                lockscreen_clock->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                lockscreen_clock->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true);

                    break;

                case WM_SET_TIMEZONE:
                WatchScreenUpdateInterval=350;
                    lockscreen_clock->setWinTextSize(1);
                    lockscreen_clock->ResizeWindow(112, 112,false);//expand for more digits, .xyz expand by  pixels
                    lockscreen_clock->MoveWindow(16,16,false);

                    break;

                case WM_APPMENU:
                WatchScreenUpdateInterval=500;
                    lockscreen_clock->setWinTextSize(1); //reduce text size to handle list on screen correctly
                    lockscreen_clock->ResizeWindow(128, 128,false);
                    lockscreen_clock->MoveWindow(0,0,true);
                   
                break;    

                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    lockscreen_clock->updateContent("ERROR: Bad Mode");
                   
                break;
            }
tft.fillScreen(tcol_background); //clean the scren up, prep 4 next udpate
}       

void loadAlarmToBuffer(uint8_t index) {
    if (index < NUM_TIMERS) {
        usralmstbuf = usrmade_timers[index];
    }
}
void saveAlarmGeneric(usr_alarm_st* array, uint8_t index) {
    // copy the working buffer into your target array slot
    array[index] = usralmstbuf;
}

extern usr_alarm_st usrmade_alarms[10]; 
extern usr_alarm_st usrmade_timers[5];
bool CacheMenuConfirmState = false; 


void watchscreen(void *pvParameters) { 
    (void)pvParameters;


    //update rate changes per device yammering


    for (;;) {
        if (IsScreenOn && lockscreen_clock) {
            unsigned long now = millis();

            switch (currentWatchMode) {

                case WM_MAIN:
                WatchScreenUpdateInterval=500;
                    snprintf(watchscreen_buf, sizeof(watchscreen_buf), "%02d:%02d<textsize(2)>:%02d<n><textsize(1)>%s %d",
                             CurrentNormieTime.hour,
                             CurrentNormieTime.minute,
                             CurrentNormieTime.second,
                             TRIchar_month_names[CurrentNormieTime.month],
                             CurrentNormieTime.day);
                    lockscreen_clock->updateContent(watchscreen_buf);

                    snprintf(thermoStr, sizeof(thermoStr), "%dC", temp_c);
                    lockscreen_thermometer->updateContent(thermoStr);

                    snprintf(hrStr, sizeof(hrStr), "%dbpm", AVG_HR);
                    lockscreen_biomon->updateContent(hrStr);
                    break;

                case WM_STOPWATCH: {
                    WatchScreenUpdateInterval=200;//update WAY more frequently at 200ms
                    unsigned long elapsed;
                    if (stopwatchRunning) {
                        elapsed = stopwatchElapsed + (now - stopwatchStart);
                    } else {
                        elapsed = stopwatchElapsed;
                    }
                    
                    unsigned int s  = (elapsed / 1000) % 60;
                    unsigned int m  = (elapsed / 60000) % 60;
                    unsigned int h  = elapsed / 3600000;
                    unsigned int ms = elapsed % 1000;

                   snprintf(watchscreen_buf, sizeof(watchscreen_buf), "%02u:%02u:%02u<n><textsize(1)>.%03u%s", h, m, s, ms, stopwatchRunning ? "<n><textsize(1)>RUN" : "<n><textsize(1)>STOP");



                    lockscreen_clock->updateContent(watchscreen_buf);//windowManagerInstance->UpdateAllWindows(true,false);
                    break;
                }

                case WM_ALARMS:
                
                    // TODO: Display upcoming alarms or alarm setup screen
                    lockscreen_clock->updateContent("ALARM MODE");//windowManagerInstance->UpdateAllWindows(true,false);
                    break;

case WM_TIMER:
    render_timer_screen();
    break;



                case WM_NTP_SYNCH:
                    lockscreen_clock->updateContent("Syncing Time...");//windowManagerInstance->UpdateAllWindows(true,false);
                    break;

                case WM_SET_TIME:
                    lockscreen_clock->updateContent("Set Time Mode");//windowManagerInstance->UpdateAllWindows(true,false);
                    break;

                case WM_SET_TIMEZONE:
                    lockscreen_clock->updateContent("Set TZ Mode");
                    windowManagerInstance->UpdateAllWindows(true,false);
                    break;


                case WM_APPMENU:
                    updateAppList(buf_applist, sizeof(buf_applist), appNames, APP_COUNT, AppMenuSelectedIndex);
                    lockscreen_clock->updateContent(buf_applist);
                 break;





                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    lockscreen_clock->updateContent("ERROR: Bad Mode");
                    break;
            }//switch statement
       // lockscreen_clock->WinDraw();

        //if (currentWatchMode != WM_APPMENU) { 
           windowManagerInstance->UpdateAllWindows(true,false);

        clearScreenEveryXCalls(1000); //sometimes screen has weird update colisions, this resets it. sure it's spagetti and will make it stutter, but whatever man. temp only, do not use in prod. 
        updateCurrentTimeVars();
        vTaskDelay(pdMS_TO_TICKS(WatchScreenUpdateInterval));

    }//if screen is the clock


    }//for;;

}//void watch screen

//input tick============================================================================================================================


void HHMM_setter_pos_setwithbounds(bool dir) {
    if (dir) {
        currentTimerField++;
    } else {
        if (currentTimerField > 0) {
            currentTimerField--;
        } else {
            currentTimerField = 0; // clamp at 0
        }
    }

    if (currentTimerField >= 18) { // 
        currentTimerField = 0;
    }
}
// Mode-specific input handlers
static void on_wm_main_input(uint16_t key) {
    // Currently no special handling for MAIN mode
}

static void on_wm_stopwatch_input(uint16_t key) {
    switch(key) {
        case key_enter:
            if (stopwatchRunning) {
                stopwatchElapsed += millis() - stopwatchStart;
                stopwatchRunning = false;
            } else {
                stopwatchStart = millis();
                stopwatchRunning = true;
            }
            break;
        default:
            break;
    }
}

static void on_wm_appmenu_input(uint16_t key) {
    switch(key) {
        case key_enter:
            transitionApp(AppMenuSelectedIndex);
            break;
            
        case key_up:
            AppMenuSelectedIndex = (AppMenuSelectedIndex == 0) 
                ? APP_COUNT - 1 
                : AppMenuSelectedIndex - 1;
            updateAppList(buf_applist, sizeof(buf_applist), 
                         appNames, APP_COUNT, AppMenuSelectedIndex);
            lockscreen_clock->updateContent(buf_applist);
            break;
            
        case key_down:
            AppMenuSelectedIndex = (AppMenuSelectedIndex + 1) % APP_COUNT;
            updateAppList(buf_applist, sizeof(buf_applist), 
                         appNames, APP_COUNT, AppMenuSelectedIndex);
            lockscreen_clock->updateContent(buf_applist);
            break;
            
        case key_back:
            currentWatchMode = WM_MAIN;
            is_watch_screen_in_menu = false;
            WATCH_SCREEN_TRANSITION(WM_MAIN);
            break;
            
        default:
            break;
    }
}
// timer_input.c
// watch_screen.c

void render_timer_screen() {
    if (timerEditState == EDIT_OFF) {
        // List view mode - show all timers with current selection
        snprintf(watchscreen_buf, sizeof(watchscreen_buf),
               "<setcolor(0xF005)>Timers (%d/%d)<setcolor(0x07ff)><n>", 
               selectedTimerIndex + 1, NUM_TIMERS);
               
        for (int i = 0; i < NUM_TIMERS; i++) {
            char alarmBuf[20];
            snprintf(alarmBuf, sizeof(alarmBuf), "%s%02d:%02d %s<n>",
                   (i == selectedTimerIndex) ? "> " : "  ",
                   usrmade_timers[i].hours,
                   usrmade_timers[i].minutes,
                   AlarmActionNames[usrmade_timers[i].E_AlarmAction]);
            strncat(watchscreen_buf, alarmBuf, sizeof(watchscreen_buf)-strlen(watchscreen_buf)-1);
        }
        strncat(watchscreen_buf, "<n>^/v Select  <- Edit", sizeof(watchscreen_buf)-strlen(watchscreen_buf)-1);
    }
    else {
        // Edit mode - use the detailed formatAlarm view
        std::string alarmStr = formatAlarm(currentTimerField, timerEditState == EDIT_CONFIRM);
        strncpy(watchscreen_buf, alarmStr.c_str(), sizeof(watchscreen_buf)-1);
    }
    watchscreen_buf[sizeof(watchscreen_buf)-1] = '\0';
    lockscreen_clock->updateContent(watchscreen_buf);
}

void on_wm_timer_input(uint16_t key) {
    Serial.printf("wmtimer_input");
    Serial.println( key);

    if (timerEditState != EDIT_OFF) {
        // Edit Mode Handling
        switch(key) {
            case key_enter:
                if (timerEditState == EDIT_CONFIRM) {
                    saveAlarmGeneric(usrmade_timers, selectedTimerIndex);
                    timerEditState = EDIT_OFF;
                } else {
                    timerEditState = EDIT_CONFIRM;
                }
                break;
                
            case key_back:
                if (timerEditState == EDIT_CONFIRM) {
                    timerEditState = EDIT_RUNNING;
                } else {
                    timerEditState = EDIT_OFF;
                }
                break;
                
case key_up:
    selectedTimerIndex = (selectedTimerIndex - 1 + NUM_TIMERS) % NUM_TIMERS;
    loadAlarmToBuffer(selectedTimerIndex); // Load without immediate save
    break;
    
case key_down:
    selectedTimerIndex = (selectedTimerIndex + 1) % NUM_TIMERS;
    loadAlarmToBuffer(selectedTimerIndex);
    break;
                
            case key_left:
                currentTimerField = (currentTimerField > 0) ? currentTimerField - 1 : 15;
                break;
                
            case key_right:
                currentTimerField = (currentTimerField + 1) % 16;
                break;
                
            default: 
                break;
        }
    } else {
        // List Navigation Mode
        switch(key) {
            case key_enter:
                timerEditState = EDIT_RUNNING;
                currentTimerField = 0;
                loadAlarmToBuffer(selectedTimerIndex); // Load selected alarm into edit buffer
                break;
                
            case key_back:
                currentWatchMode = WM_MAIN;
                WATCH_SCREEN_TRANSITION(WM_MAIN);
                break;
                
            case key_up:
                if (selectedTimerIndex > 0) {
                    selectedTimerIndex--;
                } else {
                    selectedTimerIndex = NUM_TIMERS - 1;
                }
                saveAlarmGeneric(usrmade_timers, selectedTimerIndex); // Save current
                loadAlarmToBuffer(selectedTimerIndex); // Load new
                break;
                
            case key_down:
                selectedTimerIndex = (selectedTimerIndex + 1) % NUM_TIMERS;
                saveAlarmGeneric(usrmade_timers, selectedTimerIndex); // Save current
                loadAlarmToBuffer(selectedTimerIndex); // Load new
                break;
                
            default: 
                break;
        }
    }
        render_timer_screen(); // Update display after any input
}
/* erememmemmrebbr
  EDIT_OFF,        // just display list or clock
  EDIT_RUNNING,    // dialing fields (currentTimerField 0–15)
  EDIT_CONFIRM     // “Save / Cancel” screen
} EditState;*/

// Unified input handler
void Input_handler_fn_main_screen(uint16_t key) {
    // Handle global navigation keys
        Serial.printf("main_screen_input");
    Serial.println(key);
    
    // Special case: Block global nav keys when timer is editing
    if (currentWatchMode == WM_TIMER && timerEditState != EDIT_OFF) {
        if (key == key_left || key == key_right) {
            on_wm_timer_input(key); // Let timer handle these
            return;
        }
    }

   switch(key) {
        case key_left:
            watchModeIndex = (watchModeIndex == 0) ? WM_COUNT - 1 : watchModeIndex - 1;
            currentWatchMode = (WatchMode)watchModeIndex;
            WATCH_SCREEN_TRANSITION(currentWatchMode);
            return;
            
        case key_right:
            watchModeIndex = (watchModeIndex + 1) % WM_COUNT;
            currentWatchMode = (WatchMode)watchModeIndex;
            WATCH_SCREEN_TRANSITION(currentWatchMode);
            return;

        //todo have popup menu or whatever here, with actual window show 

  case key_up:
    strip.setPixelColor(0, strip.Color(255, 255, 255)); // White
    strip.show();
    Serial.println("Flashlight ON");
    break;
    
case key_down:
    strip.setPixelColor(0, strip.Color(0, 0, 0)); // Off
    strip.show();
    Serial.println("Flashlight OFF");
    break;

         default:
         break;   
   }
    // Route to mode-specific handler
    switch(currentWatchMode) {
        case WM_MAIN:        on_wm_main_input(key); break;
        case WM_STOPWATCH:   on_wm_stopwatch_input(key); break;
        case WM_APPMENU:     on_wm_appmenu_input(key); break;
        case WM_TIMER:       on_wm_timer_input(key); break;
        // Add stubs for other modes
        case WM_ALARMS:      /* Implement later */ break;
        case WM_NTP_SYNCH:   /* Implement later */ break;
        case WM_SET_TIME:    /* Implement later */ break;
        case WM_SET_TIMEZONE:/* Implement later */ break;
        default: break;
    }
}

// input_task.c
void INPUT_tick(void *pvParameters) {
    S_UserInput uinput;
    uint32_t lastInputTime = 0;
    const TickType_t xDelay = pdMS_TO_TICKS(10);

    for (;;) {
        while (xQueueReceive(processInputQueue, &uinput, 0) == pdPASS) {
            if (!uinput.isDown || (millis() - lastInputTime < 150)) {
                continue;  // Skip releases and debounce
            }
            lastInputTime = millis();

            switch (CurrentOpenApplicationIndex) {
                case APP_LOCK_SCREEN:
                    Input_handler_fn_main_screen(uinput.key);
                    break;
                case APP_HEALTH:
                    // Health app input handling
                    break;
                default:
                    break;
            }
        }

        // Purge if queue overflow
        if (uxQueueMessagesWaiting(processInputQueue) > 10) {
            xQueueReset(processInputQueue);
        }

        updateHRsensor();
        PollEncoders();
        PollButtons();
        vTaskDelay(xDelay);
    }
}



//app health monitor================================================================================================================================================
std::shared_ptr<Window> hrmonitor;

WindowCfg w_conf_hrmon = {
  0, 0, 128, 128,
  false, false,
  2, true,
  tcol_secondary, tcol_background, tcol_primary,
  1000
};

void CREATE_Healthmonitor_WINDOWS() {
  hrmonitor = std::make_shared<Window>("hrmonitor", w_conf_hrmon, "x");
  windowManagerInstance->registerWindow(hrmonitor);
  DBG_PRINTLN("HR Monitor OK");
}

void HR_MONITOR_SCREEN_TRANSITION(HealthmonitorMode desiredMode) {
  switch (desiredMode) {
    case HMM_BIOMONITOR:
      hrmonitor->updateContent("Mode: Biomonitor");
      break;
    case HMM_DAYHISTORY:
      hrmonitor->updateContent("Mode: Day History");
      break;
    case HMM_HISTORY:
      hrmonitor->updateContent("Mode: History");
      break;
    case HMM_SETTINGS:
      hrmonitor->updateContent("Mode: Settings");
      break;
    default:
      Serial.println("Unknown HealthMonitorMode!");
      hrmonitor->updateContent("ERROR: Bad Mode");
      break;
  }
}

void healthMonitorTask(void *pvParameters) {
  HealthmonitorMode currentMode = HMM_BIOMONITOR;
  unsigned long lastUpdate = millis();

  while (1) {
    if (IsScreenOn && hrmonitor) {
      // Do your reading here
      HR_MONITOR_SCREEN_TRANSITION(currentMode);
    }

    clearScreenEveryXCalls(1000);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// app nfc===================================================================================================================================================================================// NFC globals
std::shared_ptr<NFCManager> nfc_manager; // Keeps alive
TaskHandle_t nfcTaskHandle = NULL;
static std::shared_ptr<Window> nfcStatusWindow;

WindowCfg nfcStatusCfg = {
  10, 50, 100, 20, false, false, 1, true,
  tcol_secondary, tcol_background, tcol_primary,
  500
};

void setupNFCUI() {
  nfcStatusWindow = std::make_shared<Window>("nfc_app", nfcStatusCfg, "NFC: OFF");
  windowManagerInstance->registerWindow(nfcStatusWindow);
 nfc_manager = std::make_shared<NFCManager>(IRQ, NFC_RST_PIN);

}

void start_nfc_task() {
  if (nfc_manager == nullptr) {
    Serial.println("[ERROR] NFC Manager is null!");
    return;
  }

  xTaskCreatePinnedToCore(
    nfcTask,
    "NFCTask",
    8192,
    nfc_manager.get(),  // Pass raw pointer
    1,
    &nfcTaskHandle,
    1
  );
}

void nfcTask(void *pvParameters) {
  NFCManager* local_manager = static_cast<NFCManager*>(pvParameters);
  uint8_t uid[7];
  uint8_t uidLength;
  char NFC_filepath[80];

  while (1) {
    Serial.println("[NFC] Scanning...");

    bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200);

    if (!success) {
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    Serial.print("[NFC] Found Tag UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.printf("%02X ", uid[i]);
    }
    Serial.println();

    snprintf(NFC_filepath, sizeof(NFC_filepath), "/nfc");
    if (!SD.exists(NFC_filepath)) {
      SD.mkdir(NFC_filepath);
    }

    snprintf(NFC_filepath, sizeof(NFC_filepath),
             "/nfc/%02X%02X%02X%02X%02X%02X%02X.nfcdat",
             uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);

    File dataFile = SD.open(NFC_filepath, FILE_WRITE);
    if (!dataFile) {
      Serial.println("[SD] Couldn't open file to write");
    } else {
      Serial.println("[SD] Writing tag data...");

      uint8_t data[4];
      for (uint8_t page = 0; page < 42; page++) {
        success = nfc.ntag2xx_ReadPage(page, data);
        if (success) {
          dataFile.printf("PAGE %02d: %02X %02X %02X %02X\n",
                          page, data[0], data[1], data[2], data[3]);
        } else {
          dataFile.printf("PAGE %02d: Read Error\n", page);
        }
      }
      dataFile.close();
      Serial.printf("[SD] Saved: %s\n", NFC_filepath);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void NFC_SCREEN_TRANSITION(NFC_MODE desiredMode) {
  switch (desiredMode) {
    case NFC_OFF: nfcStatusWindow->updateContent("NFC: rdy"); break;
    case NFC_PLAYBACK: nfcStatusWindow->updateContent("NFC: PLAYBACK"); break;
    case NFC_RECORD: nfcStatusWindow->updateContent("NFC: RECORDING"); break;
  }

  if (nfc_manager) {
    nfc_manager->setMode(desiredMode);
  }
}

void end_nfc_task() {
  if (nfcTaskHandle) {
    vTaskDelete(nfcTaskHandle);
    nfcTaskHandle = NULL;
  }
}


//app settings [dogshit gear icon,general settings]================================================================================================================================================
//=================================================================================================================================================================================================
/*
    GSLC_POWER,         // sleep modes
    GSLC_ALERTS,        // notifications, alarms
    GSLC_DISPLAY,       // screen settings
    GSLC_DATA,          // storage, sd card
    GSLC_WIRELESS,      // wifi, bt
    GSLC_EXT_HARDWARE,  // modules, sensors
    GSLC_CATEGORY_COUNT
*/



// mode_select_settings function with empty switch
void mode_select_settings(GlobalSettingsListCategory category) {
    switch (category) {
        case GSLC_POWER:
            break;
        case GSLC_ALERTS:
            break;
        case GSLC_DISPLAY:
            break;
        case GSLC_DATA:
            break;
        case GSLC_WIRELESS:
            break;
        case GSLC_EXT_HARDWARE:
            break;
        default:
            break;
    }
}


void mode_select_settings(GlobalSettingsListCategory category, GlobalSettings* settings) {
    switch (category) {
        case GSLC_POWER:
            // Example usage
            //settings->power.mode = PWR_ULTRA;
            break;

        case GSLC_ALERTS:
            // Example usage
            
            settings->alerts.intensity = 200;
            settings->alerts.flash_light = 1;
            settings->alerts.source = ALERT_SRC_PHONE;
            
            break;

        case GSLC_DISPLAY:
            // Example usage 
            
            settings->display.brightness = 180;
            settings->display.greyscale = 0;
            settings->display.fast_refresh = 1;
            
            break;

        case GSLC_DATA:
            // Example usage

            break;

        case GSLC_WIRELESS:
            // Example usage
           // settings->wireless.nfc_enabled = 1;
          //  settings->wireless.wifi_enabled = 1;
          //  settings->wireless.bt_enabled = 0;
            break;

        case GSLC_EXT_HARDWARE:
            // Fill this later
            break;

        default:
            break;
    }
}


/*
void nfc_task(void *pvParameters) {
    // "Constructor" code - runs once when task starts
    // Initialize NFC hardware (turn on power pin, etc.)
    FGPIO_HIGH(PWR_NFC);
    //nfc_init();  // Or whatever your initialization function is
    
    // Main task loop
    for (;;) {
        // Your normal task processing here
        vTaskDelay(pdMS_TO_TICKS(100));  // Example delay
    }
    
    // Note: Code here would normally never run because of the infinite loop
    // But if you have a loop condition or break from the loop:
    
    // "Destructor" code - runs when task is about to exit
    FGPIO_LOW(PWR_NFC);  // Turn off NFC power
   // nfc_deinit();  // Clean up NFC resources if needed
    
    // Task must self-delete if it exits its function
    vTaskDelete(NULL);
}



void task_main_menu(void *pvParameters) {
    // "Constructor" code - runs once when task starts
    enum mainmenumode{
        
    }; //idk fuck you it's a temp. rn we'll do text shit for now
   
    // Main task loop
    for (;;) {
        // Your normal task processing here
        vTaskDelay(pdMS_TO_TICKS(100));  // Example delay
    }
    
    // Note: Code here would normally never run because of the infinite loop
    // But if you have a loop condition or break from the loop:
    

    // Task must self-delete if it exits its function
    vTaskDelete(NULL);
}
*/



//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


std::shared_ptr<Window> IR_REMOTE_WIN; //declare existance of the window for the application

WindowCfg w_conf_IR_REMOTE = { //hr mon
    0, 0, //xy
    128 , 128, //wh
    false, false, //auto align,wraptext
    1, //text size
    true,//borderless?
    tcol_secondary, tcol_background, tcol_primary, // <-- pass addresses!. colors
    1000 //update interval ms
};

//guess what buddy i dont wanna program this rn





void loop() { } //i remember when merely this was how arduino code was written before freerots. we'd have to setup non blocking delay spagetti. 
// you could only do one thing at once that way, and it was funny. this was back when i wanted to make a "smart" watch when i was a teenager, but didn't fully-ass it like i'm doing right now
