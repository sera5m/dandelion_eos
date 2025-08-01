//include define order
    //Arduino core headers

   // System headers

    //Third-party libraries

    // local headers

//arduino compats
#include <Wire.h>
#include <time.h>
#include <stdio.h>
//do not touch
#include "Wiring.h" //my hardware definitions-comes first

// Arduino core headers
#include <Wire.h>
#include <SPI.h>
#include <SD.h>             // esp specific lib
#include <WiFi.h>
#include <time.h>
#include <HardwareSerial.h>

// System headers
#include <sstream>
#include <cstring>
#include <cstddef>
#include <stdbool.h>
#include <cstdlib>  // For rand()
#include <ctime>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <atomic>
#include <stdio.h>

// Third-party libraries
#include "USB.h"
#include "USBCDC.h"
#include <esp32-hal-spi.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <pgmspace.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Adafruit_NeoPixel.h>
#include "esp_pm.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//req for most things

//mathematics prerequisite
#include "types.h"


#include "inputHandler.h"
#include "mdl_clock.h"
#include "SDFS.h"
#include "IR_Remote.ino"
#include "Micro2D_A.ino"
//apps
#include "MainApp.h"
#include "NFCAPP.h"
// Hardware SPI bus instance
SPIClass spiBus(HSPI);

// PCNT unit assignments
#define PCNT_UNIT_X PCNT_UNIT_0
#define PCNT_UNIT_Y PCNT_UNIT_1

// Encoder channel assignments
#define PCNT_CHANNEL_X PCNT_CHANNEL_0
#define PCNT_CHANNEL_Y PCNT_CHANNEL_0  // Separate unit gets its own channel 0



typedef enum {
    TIMER_FIELD_HOUR_TENS = 0,   // First digit of hour [0-2]
    TIMER_FIELD_HOUR_ONES,       // Second digit of hour [0-9] (0-3 if tens=2)
    TIMER_FIELD_MIN_TENS,        // First digit of minute [0-5]
    TIMER_FIELD_MIN_ONES,        // Second digit of minute [0-9]
    TIMER_FIELD_ALARM_ACTION,    // Alarm type selection
    TIMER_FIELD_SNOOZE,          // Snooze duration [1-30]
    TIMER_FIELD_COUNT            // Total fields
} TimerField;

typedef enum {
    ALARM_FIELD_HOUR_TENS,   // First digit of hour [0-2]
    ALARM_FIELD_HOUR_ONES,       // Second digit of hour [0-9] (0-3 if tens=2)
    ALARM_FIELD_MIN_TENS,        // First digit of minute [0-5]
    ALARM_FIELD_MIN_ONES,        // Second digit of minute [0-9]
    ALARM_FIELD_ALARM_ACTION,    // Alarm type selection
    ALARM_FIELD_SNOOZE,          // Snooze duration [1-30]
    ALARM_FIELD_DAY_MON,         // Day selection starts here
    ALARM_FIELD_DAY_TUE,
    ALARM_FIELD_DAY_WED,
    ALARM_FIELD_DAY_THU,
    ALARM_FIELD_DAY_FRI,
    ALARM_FIELD_DAY_SAT,
    ALARM_FIELD_DAY_SUN,
    ALARM_FIELD_COUNT            // Total fields
} ALARMField;
// Clamp macro

// Global nav position
/*
extern int16vect globalNavPos = {0, 0, 0};
extern bool ShouldNavWrap = true;           // wrap or clamp
extern int16vect Navlimits_ = {64, 64, 0};//xyz, just as example here init'd to 64 of each
*/
//input handler.h has these

// Wrap helper
static inline int16_t wrap_value(int16_t value, int16_t min, int16_t max) {
    int16_t range = max - min + 1;
    if (range <= 0) return min;  // Defensive
    while (value < min) value += range;
    while (value > max) value -= range;
    return value;
}



#define WATCHSCREEN_BUF_SIZE 512

template<typename T>
inline T CLAMP(const T& val, const T& min, const T& max) {
    return (val < min) ? min : (val > max) ? max : val;
}

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


//#include "watch_Settings.h" //configuration file for settings


//heartrate
#include "MAX30105.h" //sparkfun lib
#include "heartRate.h"
#include "spo2_algorithm.h"//aw lawd
#include "mdl_heartmonitor.h" //my bad code
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

static std::shared_ptr<Window> Win_GeneralPurpose; QueueHandle_t lockscreenQueue = nullptr;
static std::shared_ptr<Window> lockscreen_biomon;
static std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;

// timer_editor.h


#define NUM_TIMERS 5  // Replace magic number
#define NUM_TIMER_FIELDS 2  // For HH:MM editing

 EditState timerEditState;
 uint8_t currentTimerField;  





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
    color[0] = (input >> 16) & 0xFF; // Red component
    color[1] = (input >> 8) & 0xFF;  // Green component
    color[2] = input & 0xFF;         // Blue component

    // Extract last 8 bits (most significant byte)
    *last8 = (input >> 24) & 0xFF;   // Days bitmask
}

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

uint8_t selectedTimerIndex = 0;  // Tracks which timer we're editing

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
    
   // if (initSDCard()){        DBG_PRINTLN("SD Card initialized.");}else{DBG_PRINTLN("SD Card failed!");} 


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
    4096,               // Stack size-potentially larger to handle things?
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
    char watchscreen_buf[WATCHSCREEN_BUF_SIZE];
    char thermoStr[8];
    char hrStr[8];
    
uint8_t watchModeIndex = 0; //persistant var, COMPLETELY unrelated from mouse, ONLY indicates the watch mode itself

  
  //need struct for theother thing here for alarm mode

// Field ordering matching formatTimerSetter() display


// Modified handleTimerFieldAdjustment
void handleTimerFieldAdjustment(bool increase) {
    // Safety check - ensure we're editing a valid timer
    if (globalNavPos.y >= NUM_TIMERS) return;
    
    // Get reference to the timer we're editing
    usr_alarm_st& currentAlarm = usrmade_timers[globalNavPos.y];
    
    const int8_t direction = increase ? 1 : -1;
    uint8_t days_bitmask;
    uint8_t color[3];
    
    split_u32_to_24(currentAlarm.LightColor, &days_bitmask, color);

    switch (currentTimerField) {
        case TIMER_FIELD_HOUR_TENS: {
            int tens = currentAlarm.hours / 10;
            tens = (tens + direction + 3) % 3; // Wrap 0-2
            int ones = min(currentAlarm.hours % 10, tens == 2 ? 3 : 9);
            currentAlarm.hours = tens * 10 + ones;
            break;
        }
        
        case TIMER_FIELD_HOUR_ONES: {
            int tens = currentAlarm.hours / 10;
            int max_ones = (tens == 2) ? 3 : 9;
            int ones = (currentAlarm.hours % 10 + direction + max_ones + 1) % (max_ones + 1);
            currentAlarm.hours = tens * 10 + ones;
            break;
        }
        
        // ... other cases similarly updated ...
        
        case TIMER_FIELD_ALARM_ACTION: {
            int action = (static_cast<int>(currentAlarm.E_AlarmAction) + direction);
            action = (action + ALARM_ACTION_MAX) % ALARM_ACTION_MAX;
            currentAlarm.E_AlarmAction = static_cast<alarmAction>(action);
            break;
        }
        
        // Recombine LightColor
        currentAlarm.LightColor = ((uint32_t)days_bitmask << 24) | 
                                (color[0] << 16) | 
                                (color[1] << 8) | 
                                color[2];
    }
}


std::string formatTimerSetter(uint8_t highlightedField, bool confirmMode, uint8_t timerIndex) {
    // Validate timer index first
    if (timerIndex >= NUM_TIMERS) {
        return "Invalid timer";
    }

    // Get reference to the current timer
    usr_alarm_st& currentTimer = usrmade_timers[timerIndex];
    uint8_t days_bitmask;
    uint8_t color[3];
    split_u32_to_24(currentTimer.LightColor, &days_bitmask, color);

    std::ostringstream oss;
    
    // 1. Format Time Section
    oss << "Create new timer: <n>Duration: ";
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", currentTimer.hours, currentTimer.minutes);
    
    // Highlight individual time components
    for (uint8_t i = 0; i < 5; i++) {
        if (i == 2) { // Colon separator
            oss << timeStr[i];
            continue;
        }
        
        bool shouldHighlight = false;
        switch (i) {
            case 0: shouldHighlight = (highlightedField == TIMER_FIELD_HOUR_TENS); break;
            case 1: shouldHighlight = (highlightedField == TIMER_FIELD_HOUR_ONES); break;
            case 3: shouldHighlight = (highlightedField == TIMER_FIELD_MIN_TENS); break;
            case 4: shouldHighlight = (highlightedField == TIMER_FIELD_MIN_ONES); break;
        }
        
        if (shouldHighlight) {
            oss << "<setcolor(0xF005)>[" << timeStr[i] << "]<setcolor(0x07ff)>";
        } else {
            oss << timeStr[i];
        }
    }

    // 2. Format Alarm Action
    oss << "<n><n>Alert: ";
    if (highlightedField == TIMER_FIELD_ALARM_ACTION) {
        oss << "<setcolor(0xF005)>[" << AlarmActionNames[currentTimer.E_AlarmAction] << "]<setcolor(0x07ff)>";
    } else {
        oss << AlarmActionNames[currentTimer.E_AlarmAction];
    }

    // 3. Format Snooze Duration
    oss << "<n><n>Snooze: ";
    if (highlightedField == TIMER_FIELD_SNOOZE) {
        oss << "<setcolor(0xF005)>[" << static_cast<int>(currentTimer.SnoozeDur) << "min]<setcolor(0x07ff)>";
    } else {
        oss << static_cast<int>(currentTimer.SnoozeDur) << "min";
    }

    // 4. Format Action Buttons
    oss << "<n><n>enter/back";
    if (confirmMode) {
        oss << "<setcolor(0xF005)>[SAVE?]";
    } else {
        oss << "<setcolor(0xF005)>[edit]";
    }

    return oss.str();
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
        Win_GeneralPurpose = std::make_shared<Window>("Win_GeneralPurpose", d_ls_c_cfg, "HH:MM:SS");
    windowManagerInstance->registerWindow(Win_GeneralPurpose);
    DBG_PRINTLN("Clock OK");

            lockscreen_biomon = std::make_shared<Window>("lockscreen_biomon", d_ls_b_cfg, "XXXbpm");
    windowManagerInstance->registerWindow(lockscreen_biomon);
    DBG_PRINTLN("Biomon OK");

        lockscreen_thermometer = std::make_shared<Window>("lockscreen_thermometer", d_ls_th_cfg, "XXXC");
    windowManagerInstance->registerWindow(lockscreen_thermometer);

}
//i put this above everything to avoid bugs because .ino is evil. typedef enum{WM_MAIN, WM_STOPWATCH,WM_ALARMS,WM_TIMER,WM_NTP_SYNCH, WM_SET_TIME,WM_SET_TIMEZONE}Wat ch mo de enum
//app-appmenu==============================================================================
  void transitionApp(uint8_t index) {
    AppName app = (AppName)index;
rst_nav_pos(); //reset mouse pos between apps
    switch (app) {
        //set 
        CurrentOpenApplicationIndex=app; //set via lazymaxxing

        case APP_LOCK_SCREEN:
            // Do something for lock screen
            CurrentOpenApplicationIndex=APP_LOCK_SCREEN; //note! need to set this on sucess, move this later to outside just this mode. this is the current open app n should only be set on success!
            break;
        case APP_HEALTH:
            // Do something for health app
            break;
        case APP_NFC:
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
//globalNavPos.x

void updateAppList(char *buf, size_t bufSize, const char **apps, int count) { //part of appmenu
    buf[0] = '\0';
    for (int i = 0; i < min(count, MAX_VISIBLE); ++i) {
        if (i == globalNavPos.y) {  // Now checking y position
            strncat(buf, "<setcolor(0xdbbf)>[", bufSize);
            strncat(buf, apps[i], bufSize);
            strncat(buf, "]<setcolor(0x07ff)>", bufSize);
        } else {
            strncat(buf, apps[i], bufSize);
        }
        strncat(buf, "<n>", bufSize);
    }//end for, also i should not use for lol


// Pad with blank lines if needed
int visibleLines = APP_COUNT < MAX_VISIBLE ? APP_COUNT : MAX_VISIBLE;
 for (int i = visibleLines; i < MAX_VISIBLE; ++i) {
  strncat(buf, "<n>", bufSize - strlen(buf) - 1);
}

    // Optionally ensure trailing newline if needed
    //strncat(buf, "<n>", bufSize - strlen(buf) - 1);
//end fn update applist

}//end fn



WatchMode currentWatchMode = WM_MAIN;
int stopwatchElapsed=0;

void WATCH_SCREEN_TRANSITION(WatchMode desiredMode){
   rst_nav_pos();//reset mouse
  Win_GeneralPurpose->updateContent("");//remove content, avoid visual only bug

switch (desiredMode){
  
//tft.fillScreen(tcol_background);//clean everything

                case WM_MAIN:
            
            Navlimits_ = {0, 0, 0};
                WatchScreenUpdateInterval=500;
                //update bg? 
                //Win_GeneralPurpose->updateContent("");//fixes bug with text overflow
                Win_GeneralPurpose->setWinTextSize(2);
                Win_GeneralPurpose->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true); //reset to original config size reguardless of original config
                
                break;
                
                case WM_STOPWATCH:
                    Navlimits_ = {0, 1, 0}; //there's nothing to nav here, just enter and such
                    WatchScreenUpdateInterval=120;//update more frequently. unfortunately, there's still an issue with latency so we'll keep 
                    //Win_GeneralPurpose->updateContent("");//fixes bug with text overflow
                    Win_GeneralPurpose->setWinTextSize(2);
                    Win_GeneralPurpose->ResizeWindow(128, d_ls_c_cfg.height,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x-14, d_ls_c_cfg.y,true);//move 14 pixels to the left to offset more digits.
                                                        //x now has effective increased size of 28px. probably better if i did this as a new struct but who cares. 
                         //resize window and force update as of 6/25/25 have the ability to not force the screen to update, preventing graphical glitches
                 
                 break;
                

                case WM_ALARMS:
                Navlimits_ = {0, 1, 0};//expand nav limits later navlimits
                    // TODO: Display upcoming alarms or alarm setup screen
                    WatchScreenUpdateInterval=350;
                    Win_GeneralPurpose->setWinTextSize(1);
                    Win_GeneralPurpose->ResizeWindow(128, 112,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(0,16,false); //xy

                 break;

                case WM_TIMER:
                Navlimits_ = {0, NUM_TIMERS-1, 0};//this needs to change inside the timer itself
                WatchScreenUpdateInterval=350;
                Win_GeneralPurpose->setWinTextSize(1);
               Win_GeneralPurpose->ResizeWindow(128, 112,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(0,16,false);
                  break;

                case WM_NTP_SYNCH:
                WatchScreenUpdateInterval=500;
                Win_GeneralPurpose->setWinTextSize(2);
                Win_GeneralPurpose->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,false);
                 Win_GeneralPurpose->updateContent("time synch, wifi");

                  break;

                case WM_SET_TIME:
                WatchScreenUpdateInterval=500;
                Win_GeneralPurpose->setWinTextSize(2);
                Win_GeneralPurpose->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true);

                    break;

                case WM_SET_TIMEZONE:
                WatchScreenUpdateInterval=350;
                    Win_GeneralPurpose->setWinTextSize(1);
                    Win_GeneralPurpose->ResizeWindow(112, 112,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(16,16,false);

                    break;

                case WM_APPMENU:
                Navlimits_ = {0, 1, 0};
                WatchScreenUpdateInterval=500;
                     Navlimits_ = {0, APP_COUNT-1, 0};
                    Win_GeneralPurpose->setWinTextSize(1); //reduce text size to handle list on screen correctly
                    Win_GeneralPurpose->ResizeWindow(128, 128,false);
                    Win_GeneralPurpose->MoveWindow(0,0,true);
                   
                break;    

                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    Win_GeneralPurpose->updateContent("ERROR: Bad Mode");
                   
                break;
            }//end switch
tft.fillScreen(tcol_background); //clean the scren up, prep 4 next udpate
}//end fn       





extern usr_alarm_st usrmade_alarms[10]; 
extern usr_alarm_st usrmade_timers[5];
bool CacheMenuConfirmState = false; 


void watchscreen(void *pvParameters) { 
    (void)pvParameters;


    //update rate changes per device yammering


    for (;;) {
        if (IsScreenOn && Win_GeneralPurpose) {
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
                    Win_GeneralPurpose->updateContent(watchscreen_buf);

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



                    Win_GeneralPurpose->updateContent(watchscreen_buf);//windowManagerInstance->UpdateAllWindows(true,false);
                    break;
                }

                case WM_ALARMS:
                
                    // TODO: Display upcoming alarms or alarm setup screen
                    Win_GeneralPurpose->updateContent("ALARM MODE");//windowManagerInstance->UpdateAllWindows(true,false);
                    break;

case WM_TIMER:
    render_timer_screen();
    break;



                case WM_NTP_SYNCH:
                    Win_GeneralPurpose->updateContent("Syncing Time...");//windowManagerInstance->UpdateAllWindows(true,false);
                    break;

                case WM_SET_TIME:
                    Win_GeneralPurpose->updateContent("Set Time Mode");//windowManagerInstance->UpdateAllWindows(true,false);
                    break;

                case WM_SET_TIMEZONE:
                    Win_GeneralPurpose->updateContent("Set TZ Mode");
                    windowManagerInstance->UpdateAllWindows(true,false);
                    break;


                case WM_APPMENU:
                    updateAppList(buf_applist, sizeof(buf_applist), appNames, APP_COUNT);
                    Win_GeneralPurpose->updateContent(buf_applist);
                 break;





                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    Win_GeneralPurpose->updateContent("ERROR: Bad Mode");
                    break;
            }//switch statement
       // Win_GeneralPurpose->WinDraw();

        //if (currentWatchMode != WM_APPMENU) { 
           windowManagerInstance->UpdateAllWindows(true,false);

        clearScreenEveryXCalls(1000); //sometimes screen has weird update colisions, this resets it. sure it's spagetti and will make it stutter, but whatever man. temp only, do not use in prod. 
        
        vTaskDelay(pdMS_TO_TICKS(WatchScreenUpdateInterval));

    }//if screen is the clock


    }//for;;

}//void watch screen

//input tick============================================================================================================================


// Navigation dimensions


// Navigation dimensions
// Basic vector type

// The clean version of your function
// Template CLAMP function (put this at file/header scope)


// Updated changeNavPos function
void changeNavPos(int16vect input, bool wrap, int16vect navLimits) {
    if (wrap) {
        globalNavPos.x = wrap_value(globalNavPos.x + input.x, 0, navLimits.x);
        globalNavPos.y = wrap_value(globalNavPos.y + input.y, 0, navLimits.y);
        globalNavPos.z = wrap_value(globalNavPos.z + input.z, 0, navLimits.z);
    } else {
        globalNavPos.x = CLAMP<int16_t>(globalNavPos.x + input.x, 0, navLimits.x);
        globalNavPos.y = CLAMP<int16_t>(globalNavPos.y + input.y, 0, navLimits.y);
        globalNavPos.z = CLAMP<int16_t>(globalNavPos.z + input.z, 0, navLimits.z);
    }
}

void rst_nav_pos(){ //easier than manually tpying this each time
    globalNavPos.x=0;
    globalNavPos.y=0;
    globalNavPos.z=0;
}

void onVertical_input_timer_buff_setter(bool increase, uint8_t fieldIndex, uint8_t timerIndex) {
    if (timerIndex >= NUM_TIMERS) return;
    
    usr_alarm_st& alarm = usrmade_timers[timerIndex];
    const int8_t direction = increase ? 1 : -1;

    switch(fieldIndex) {
        case TIMER_FIELD_HOUR_TENS:
            alarm.hours = (alarm.hours + (increase ? 10 : -10)) % 24;
            if (alarm.hours < 0) alarm.hours += 24;
            break;
            
        case TIMER_FIELD_HOUR_ONES:
            alarm.hours = (alarm.hours + (increase ? 1 : -1)) % 24;
            if (alarm.hours < 0) alarm.hours += 24;
            break;
            
        case TIMER_FIELD_MIN_TENS:
            alarm.minutes = (alarm.minutes + (increase ? 10 : -10)) % 60;
            if (alarm.minutes < 0) alarm.minutes += 60;
            break;
            
        case TIMER_FIELD_MIN_ONES:
            alarm.minutes = (alarm.minutes + (increase ? 1 : -1)) % 60;
            if (alarm.minutes < 0) alarm.minutes += 60;
            break;
            
        case TIMER_FIELD_ALARM_ACTION: {
            int action = static_cast<int>(alarm.E_AlarmAction) + direction;
            if (action < 0) action = ALARM_ACTION_MAX - 1;
            if (action >= ALARM_ACTION_MAX) action = 0;
            alarm.E_AlarmAction = static_cast<alarmAction>(action);
            break;
        }
            
        case TIMER_FIELD_SNOOZE:
            alarm.SnoozeDur = constrain(alarm.SnoozeDur + (increase ? 1 : -1), 1, 30);
            break;
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
            transitionApp(globalNavPos.y);
            break;
            
        case key_up:
        case key_down:
            changeNavPos(int16vect{0, (key == key_up) ? -1 : 1, 0}, 
                       true, 
                       Navlimits_);
            updateAppList(buf_applist, sizeof(buf_applist), appNames, APP_COUNT);
            Win_GeneralPurpose->updateContent(buf_applist);
            break;
            
        case key_back:
            currentWatchMode = WM_MAIN;
            is_watch_screen_in_menu = false;
            WATCH_SCREEN_TRANSITION(WM_MAIN);
            globalNavPos = {0, 0, 0};
            break;
            
        default:
            break;
    }
}
// timer_input.c
// watch_screen.c



void render_timer_screen() {
    watchscreen_buf[0] = '\0'; // Always start with empty buffer
    
    if (timerEditState == EDIT_OFF) {
        // List view mode
        snprintf(watchscreen_buf, WATCHSCREEN_BUF_SIZE,
               "<setcolor(0xF005)>Timer %d/%d<setcolor(0x07ff)><n><n>", selectedTimerIndex + 1, NUM_TIMERS);
        
        for (int i = 0; i < min(NUM_TIMERS, 5); i++) { // Limit display to 5
            char alarmBuf[30];
            snprintf(alarmBuf, sizeof(alarmBuf), "%s%02d:%02d %s<n>",
                   (i == selectedTimerIndex) ? "> " : "  ",
                   usrmade_timers[i].hours,
                   usrmade_timers[i].minutes,
                   AlarmActionNames[usrmade_timers[i].E_AlarmAction]);
            
            strncat(watchscreen_buf, alarmBuf, 
                   WATCHSCREEN_BUF_SIZE - strlen(watchscreen_buf) - 1);
        }
        
        // Only show instruction if there are timers
        if (NUM_TIMERS > 0) {
            strncat(watchscreen_buf, "<n>^/v Select  <- Edit",
                   WATCHSCREEN_BUF_SIZE - strlen(watchscreen_buf) - 1);
        }
    }
    else {
        // Edit mode
            std::string alarmStr = formatTimerSetter(currentTimerField, 
                                          timerEditState == EDIT_CONFIRM,
                                          globalNavPos.y);
        strncpy(watchscreen_buf, alarmStr.c_str(), WATCHSCREEN_BUF_SIZE-1);
    }
    
    watchscreen_buf[WATCHSCREEN_BUF_SIZE-1] = '\0';
    Win_GeneralPurpose->updateContent(watchscreen_buf);
}

void on_wm_timer_input(uint16_t key) {
    Serial.printf("Timer input: 0x%04X State: %d\n", key, timerEditState);
    
    switch(timerEditState) {
        case EDIT_OFF: {
            // List navigation mode
            Navlimits_.x = 0;
            Navlimits_.y = NUM_TIMERS-1;
            Navlimits_.z = 0;
            
            switch(key) {
                case key_enter:
                    if (NUM_TIMERS > 0) {  // Only enter edit mode if timers exist
                        timerEditState = EDIT_RUNNING;
                        globalNavPos.x = TIMER_FIELD_HOUR_TENS; // Start with hour tens
                        selectedTimerIndex = globalNavPos.y;     // Set current edit timer
                    }
                    break;
                    
                case key_back:
                    currentWatchMode = WM_MAIN;
                    WATCH_SCREEN_TRANSITION(WM_MAIN);
                    return;
                    
                case key_up:
                case key_down:
                    changeNavPos(int16vect{0, static_cast<int16_t>((key == key_up) ? -1 : 1), 0}, 
                               true, Navlimits_);
                    break;
                    
                default: 
                    break;
            }
            break;
        }

        case EDIT_RUNNING: {
            // Field editing mode
            if (selectedTimerIndex >= NUM_TIMERS) {  // Safety check
                timerEditState = EDIT_OFF;
                break;
            }

            Navlimits_.x = TIMER_FIELD_COUNT-1;
            Navlimits_.y = 0;
            Navlimits_.z = 0;
            
            switch(key) {
                case key_enter:
                    timerEditState = EDIT_CONFIRM;
                    globalNavPos.x = 0; // Select "Save" option by default
                    break;
                    
                case key_back:
                    timerEditState = EDIT_OFF;
                    break;
                    
                case key_up:
                    handleTimerFieldAdjustment(true); // Increase current field
                    break;
                    
                case key_down:
                    handleTimerFieldAdjustment(false); // Decrease current field
                    break;
                    
                case key_left:
                case key_right:
                    changeNavPos(int16vect{static_cast<int16_t>((key == key_left) ? -1 : 1), 0, 0}, 
                               true, Navlimits_);
                    currentTimerField = static_cast<TimerField>(globalNavPos.x);
                    break;
                    
                default: 
                    break;
            }
            break;
        }

        case EDIT_CONFIRM: {
            // Save confirmation mode
            Navlimits_.x = 1;  // Toggle between Save (0) and Cancel (1)
            Navlimits_.y = 0;
            Navlimits_.z = 0;
            
            switch(key) {
                case key_enter:
                    if (globalNavPos.x == 0) {  // Save selected
                        // Data is already in usrmade_timers, just persist it
                        if (SaveTimer()) {  // Implement this function
                            Serial.println("Alarms saved successfully");
                        } else {
                            Serial.println("Error saving alarms");
                        }
                    }
                    timerEditState = EDIT_OFF;
                    break;
                    
                case key_back:
                    timerEditState = EDIT_RUNNING;
                    break;
                    
                case key_left:
                case key_right:
                    changeNavPos(int16vect{static_cast<int16_t>((key == key_left) ? -1 : 1), 0, 0}, 
                               false, Navlimits_); // No wrap for confirmation
                    break;
                    
                default: 
                    break;
            }
            break;
        }
    }
    
    render_timer_screen();
}

// Helper function to persist alarms to storage
bool SaveTimer() {
    // Implementation depends on your storage system
    // Example for EEPROM:
    /*
    EEPROM.put(ALARMS_STORAGE_ADDR, usrmade_timers);
    return EEPROM.commit();
    */
    return true; // Stub implementation
}

// text ConfirmOptionTimer 
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
   // strip.setPixelColor(0, strip.Color(255, 255, 255)); strip.show();    Serial.println("Flashlight ON");
    break;
    
case key_down:
    //strip.setPixelColor(0, strip.Color(0, 0, 0));     strip.show();    Serial.println("Flashlight OFF");
    break;

         default:
         break;   
   }//hald the thing
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
        UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("INPUT_tick stack: %d\n", stackRemaining);
    S_UserInput uinput;
    uint32_t lastInputTime = 0;
    const TickType_t xDelay = pdMS_TO_TICKS(10);

    for (;;) {
        updateCurrentTimeVars(); //keep time up to date, put here because this task must allways be ready
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
                case APP_NFC:
                input_handler_fn_NFCAPP(uinput.key);
                break;   //nfc tools itself

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
