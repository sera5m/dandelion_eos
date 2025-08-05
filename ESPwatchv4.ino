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
#include "helperfunctions.h"
#include "s_hell.h"
#include "inputHandler.h"
#include "mdl_clock.h"
#include "SDFS.h"
#include "IR_Remote.ino"
#include "Micro2D_A.h"
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

#include "globals.h"

// Clamp macro

// Global nav position
/*
extern int16vect globalNavPos = {0, 0, 0};
extern bool ShouldNavWrap = true;           // wrap or clamp
extern int16vect Navlimits_ = {64, 64, 0};//xyz, just as example here init'd to 64 of each
*/
//input handler.h has these

// Wrap helper

#define DBG_PRINTLN(x)  do { \
    tft.setCursor(0, _dbg_ypos); \
    tft.setTextColor(WHITE); \
    tft.setTextSize(1); \
    tft.println(x); \
    _dbg_ypos += 10; \
} while(0)

int _dbg_ypos = 0;  // screen Y cursor


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
//extern std::unique_ptr<WindowManager> WinManagerInstance;

extern std::shared_ptr<Window> Win_GeneralPurpose; 
QueueHandle_t lockscreenQueue = nullptr;
extern std::shared_ptr<Window> lockscreen_biomon;
extern  std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;//why. just why

// timer_editor.h


 extern EditState timerEditState;
 extern uint8_t currentTimerField;  





extern uint16_t tcol_primary;
extern uint16_t tcol_secondary;
extern uint16_t tcol_tertiary;
extern uint16_t tcol_highlight;
extern uint16_t tcol_background;
//var references to set for the theme
 //   0x07ff, //teal
   // 0x77f9, //i can't find a good green
   // 0xe4ff,//lavender
   // 0xd7fd,//very light green highlight
   // 0x29e6//background
//list_Themes Current_Theme=mint; //set the current theme to a nice default




extern QueueHandle_t processInputQueue;

extern char buf_applist[]; //ext for better access

extern bool is_watch_screen_in_menu;
extern bool isConfirming;



extern uint8_t CurrentOpenApplicationIndex; //for self referential current code-original reference in s_hell


extern bool stopwatchRunning;
extern unsigned long stopwatchStart;


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


 if (!WindowManager::getInstance().initialize(true)) {
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


WindowManager::getInstance().ApplyThemeAllWindows(tcol_secondary, tcol_background, tcol_primary); //with new vars



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



    extern int WatchScreenUpdateInterval;




void clearScreenEveryXCalls(uint16_t x) {
    static uint16_t callCount = 0;
    
    callCount++;
    
    if (callCount >= x) {
        tft.fillRect(0, 0, 128, 128, tcol_background);
        callCount = 0;
    }
}

 
    // Shared buffers for display
   extern char watchscreen_buf[WATCHSCREEN_BUF_SIZE];
    char thermoStr[8];
    char hrStr[8];
    
extern uint8_t watchModeIndex; //persistant var, COMPLETELY unrelated from mouse, ONLY indicates the watch mode itself

  
  //need struct for theother thing here for alarm mode

// Field ordering matching formatTimerSetter() display


// Modified handleTimerFieldAdjustment




//to keep the apps out of main for orginization we'll just slap those motherfuckers in here for definitions

//+++++++++++++++++++++++++++++++++++++++main screen====-------------------------------------



extern WindowCfg d_ls_c_cfg;

extern WindowCfg d_ls_b_cfg;

extern WindowCfg d_ls_th_cfg;



//i put this above everything to avoid bugs because .ino is evil. typedef enum{WM_MAIN, WM_STOPWATCH,WM_ALARMS,WM_TIMER,WM_NTP_SYNCH, WM_SET_TIME,WM_SET_TIMEZONE}Wat ch mo de enum
//app-appmenu==============================================================================
  





/*

extern usr_alarm_st usrmade_alarms[10]; 
extern usr_alarm_st usrmade_timers[5];
bool CacheMenuConfirmState = false; 
*/

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



                    Win_GeneralPurpose->updateContent(watchscreen_buf);//WindowManager::getInstance().UpdateAllWindows(true,false);
                    break;
                }

                case WM_ALARMS:
                
                    // TODO: Display upcoming alarms or alarm setup screen
                    Win_GeneralPurpose->updateContent("ALARM MODE");//WindowManager::getInstance().UpdateAllWindows(true,false);
                    break;

case WM_TIMER:
    render_timer_screen();
    break;



                case WM_NTP_SYNCH:
                    Win_GeneralPurpose->updateContent("Syncing Time...");//WindowManager::getInstance().UpdateAllWindows(true,false);
                    break;

                case WM_SET_TIME:
                    Win_GeneralPurpose->updateContent("Set Time Mode");//WindowManager::getInstance().UpdateAllWindows(true,false);
                    break;

                case WM_SET_TIMEZONE:
                    Win_GeneralPurpose->updateContent("Set TZ Mode");
                    WindowManager::getInstance().UpdateAllWindows(true,false);
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
           WindowManager::getInstance().UpdateAllWindows(true,false);

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






// text ConfirmOptionTimer 
/* erememmemmrebbr
  EDIT_OFF,        // just display list or clock
  EDIT_RUNNING,    // dialing fields (currentTimerField 0–15)
  EDIT_CONFIRM     // “Save / Cancel” screen
} EditState;*/

// Unified input handler


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
  WindowManager::getInstance().registerWindow(hrmonitor);
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



/*


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


*/

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
