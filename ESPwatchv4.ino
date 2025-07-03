//do not touch
#include "Wiring.h" //my hardware definitions

//esp32-s3 hardware
#include "USB.h"
#include "USBCDC.h"
#include <Wire.h>
#include <esp32-hal-spi.h>
#include "apps.ino"//GOSHIES I WONDER WHAT COULD BE IN HERE
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

//stupid fucking debugs to make this work right
typedef enum{WM_MAIN, WM_STOPWATCH,WM_ALARMS,WM_TIMER,WM_NTP_SYNCH, WM_SET_TIME,WM_SET_TIMEZONE}WatchMode;
typedef enum{HMM_BIOMONITOR, //Current fuckshit like that beep boop beeip in hopital
HMM_DAYHISTORY,    //a bar graph over the past x days.
   HMM_HISTORY, //this month/historical trends. on long scales of time we'll just store average hr as waking/sleeping 
   HMM_SETTINGS   //idk man what do you even config here?
}HealthmonitorMode;

//#include "watch_Settings.h" //configuration file for settings

#include <SPI.h>
SPIClass spiBus(HSPI);


//esp32 specifiic
#include "esp_pm.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp_sleep.h"
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

int16_t temp_c=69;
extern int AVG_HR;

//fuckj this were gonna put it wice
// Globals.cpp or at the top of your main .ino (outside setup and loop)


bool deviceIsAwake=true;

Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST); //note: &spiBus is required to pass main spi 

//init windows
WindowManager* windowManagerInstance = nullptr;

static std::shared_ptr<Window> lockscreen_clock; QueueHandle_t lockscreenQueue = nullptr;
static std::shared_ptr<Window> lockscreen_biomon;
static std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;


#include <atomic>


uint16_t tcol_primary=0x07E0;
uint16_t tcol_secondary=0x0000;
uint16_t tcol_tertiary=0x4208;
uint16_t tcol_highlight=0xF805;
uint16_t tcol_background=0x3256;
//var references to set for the theme

//list_Themes Current_Theme=mint; //set the current theme to a nice default




extern QueueHandle_t processInputQueue;


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

void writeCommand(uint8_t cmd) {
  digitalWrite(OLED_DC, LOW);  // Command mode
  digitalWrite(SPI_CS_OLED, LOW);
  SPI.transfer(cmd);
  digitalWrite(SPI_CS_OLED, HIGH);
}

void writeData(uint8_t data) {
  digitalWrite(OLED_DC, HIGH);  // Data mode
  digitalWrite(SPI_CS_OLED, LOW);
  SPI.transfer(data);
  digitalWrite(SPI_CS_OLED, HIGH);
}

void DISPLAY_CONF() {
  SPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));

  writeCommand(SSD1351_CMD_CLOCKDIV);
  writeData(0xF1);

  writeCommand(SSD1351_CMD_MASTER_CURRENT_CONTROL);
  writeData(0x0F);

  writeCommand(SSD1351_CMD_CONTRASTABC);
  writeData(0xFF);
  writeData(0xFF);
  writeData(0xFF);

  SPI.endTransaction();
}




void setup() {

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


CREATE_LOCKSCREEN_WINDOWS();//CREATE DEFAULT LOCKSCREEN SHIT
   // DBG_PRINTLN("Thermo OK");
//SetDeviceTheme(mint);//change the color pallette refs

TFillRect(0,0,128,128,tcol_background);//black screen out


windowManagerInstance->ApplyThemeAllWindows(tcol_secondary, tcol_background, tcol_primary); //with new vars



processInputQueue = xQueueCreate(8, sizeof(S_UserInput)); //set up default que
xTaskCreate(watchscreen, "WatchScreen", 4096, NULL, 1, NULL);//core 0 watch screen 


    //DBG_PRINTLN("watchscreen task OK");
    xTaskCreatePinnedToCore(INPUT_tick, "INPUT_tick", 2048, NULL, 2, NULL, 1); //core 1 sensor updates


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

WatchMode currentWatchMode = WM_MAIN;
int stopwatchElapsed=0;

void WATCH_SCREEN_TRANSITION(WatchMode desiredMode){
switch (desiredMode){

                case WM_MAIN:
                WatchScreenUpdateInterval=500;
                //update bg? 
                lockscreen_clock->updateContent("");//fixes bug with text overflow
                lockscreen_clock->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                lockscreen_clock->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true); //reset to original config size reguardless of original config

                break;
                
                case WM_STOPWATCH:
                    WatchScreenUpdateInterval=120;//update more frequently. unfortunately, there's still an issue with latency so we'll keep 
                    lockscreen_clock->updateContent("");//fixes bug with text overflow
                    lockscreen_clock->ResizeWindow(128, d_ls_c_cfg.height,false);//expand for more digits, .xyz expand by  pixels
                    lockscreen_clock->MoveWindow(d_ls_c_cfg.x-14, d_ls_c_cfg.y,true);//move 14 pixels to the left to offset more digits.
                                                        //x now has effective increased size of 28px. probably better if i did this as a new struct but who cares. 
                         //resize window and force update as of 6/25/25 have the ability to not force the screen to update, preventing graphical glitches
                 break;
                

                case WM_ALARMS:
                    // TODO: Display upcoming alarms or alarm setup screen

                 break;

                case WM_TIMER:
                    // TODO: Display remaining timer or timer setup

                  break;

                case WM_NTP_SYNCH:

                  break;

                case WM_SET_TIME:

                    break;

                case WM_SET_TIMEZONE:

                    break;

                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    lockscreen_clock->updateContent("ERROR: Bad Mode");
                break;
            }

}       
void INPUT_tick(void *pvParameters) {
    S_UserInput uinput;

    for (;;) {
        int inputCount = 0;

        // Consume inputs once and update state
        while (xQueueReceive(processInputQueue, &uinput, 0) == pdPASS) {
            inputCount++;
            if (!uinput.isDown) continue;
            Serial.println(uinput.key);
            switch (uinput.key) {
                case key_left:
                    currentWatchMode = WM_STOPWATCH;
                    
                    WATCH_SCREEN_TRANSITION(WM_STOPWATCH);
                    break;
                case key_right:
                case key_back:
                    currentWatchMode = WM_MAIN;
                   // buf=" ";
                   WATCH_SCREEN_TRANSITION(WM_MAIN);
                    // Do NOT reset stopwatchRunning here (keep state)
                    break;
                case key_enter:
    if (currentWatchMode == WM_STOPWATCH) {
        if (stopwatchRunning) {
            stopwatchElapsed += millis() - stopwatchStart;
            stopwatchRunning = false;

           
        } else {
            stopwatchStart = millis();
            stopwatchRunning = true;
        }
    }
    break;

    default:
    break;
    }
}

// If inputCount exceeds 10, purge excess junk
if (inputCount > 10) {
    S_UserInput junk;
    while (xQueueReceive(processInputQueue, &junk, 0) == pdPASS) {}//do fuck all
    inputCount = 0;
                    }//end if input count

        updateHRsensor();
        PollEncoders();
        PollButtons();
        vTaskDelay(pdMS_TO_TICKS(10));
    }//end for
}//end input tick




void watchscreen(void *pvParameters) { 
    (void)pvParameters;

    // Shared buffers for display
    char buf[80];
    char thermoStr[8];
    char hrStr[8];
    //update rate changes per device yammering


    for (;;) {
        if (IsScreenOn && lockscreen_clock) {

            unsigned long now = millis();

            switch (currentWatchMode) {

                case WM_MAIN:
                WatchScreenUpdateInterval=500;
                    snprintf(buf, sizeof(buf), "%02d:%02d<textsize(1)>:%02d<n><textsize(2)>%s %d",
                             CurrentNormieTime.hour,
                             CurrentNormieTime.minute,
                             CurrentNormieTime.second,
                             TRIchar_month_names[CurrentNormieTime.month],
                             CurrentNormieTime.day);
                    lockscreen_clock->updateContent(buf);

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

                    snprintf(buf, sizeof(buf), "%02u:%02u:%02u<n><textsize(1)>.%03u", h, m, s, ms, stopwatchRunning ? "<n><textsize(1)>RUN" : "<n><textsize(1)>STOP");//modes not working i dunno whyyy


                    lockscreen_clock->updateContent(buf);
                    break;
                }

                case WM_ALARMS:
                    // TODO: Display upcoming alarms or alarm setup screen
                    lockscreen_clock->updateContent("ALARM MODE");
                    break;

                case WM_TIMER:
                    // TODO: Display remaining timer or timer setup
                    lockscreen_clock->updateContent("TIMER MODE");
                    break;

                case WM_NTP_SYNCH:
                    lockscreen_clock->updateContent("Syncing Time...");
                    break;

                case WM_SET_TIME:
                    lockscreen_clock->updateContent("Set Time Mode");
                    break;

                case WM_SET_TIMEZONE:
                    lockscreen_clock->updateContent("Set TZ Mode");
                    break;

                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    lockscreen_clock->updateContent("ERROR: Bad Mode");
                    break;
            }
        }
        clearScreenEveryXCalls(1000); //sometimes screen has weird update colisions, this resets it. sure it's spagetti and will make it stutter, but whatever man. temp only, do not use in prod. 
        updateCurrentTimeVars();
        vTaskDelay(pdMS_TO_TICKS(WatchScreenUpdateInterval));
    }
}

//app health monitor================================================================================================================================================
std::shared_ptr<Window> hrmonitor;

//HMM_BIOMONITOR, //Current fuckshit like that beep boop beeip in hopitalHMM_DAYHISTORY,    //a bar graph over the past x days.HMM_HISTORY, //this month/historical trends. on long scales of time we'll just store average hr as waking/sleeping HMM_SETTINGS   //idk man what do you even config here?
//moved to the before thing


WindowCfg w_conf_hrmon = { //hr mon
    0, 0, //xy
    128 , 128, //wh
    false, false, //auto align,wraptext
    2, //text size
    true,//borderless?
    tcol_secondary, tcol_background, tcol_primary, // <-- pass addresses!. colors
    1000 //update interval ms
};


void CREATE_Healthmonitor_WINDOWS(){
    hrmonitor = std::make_shared<Window>("hrmonitor", w_conf_hrmon, "x");
    windowManagerInstance->registerWindow(hrmonitor);
    DBG_PRINTLN("Clock OK");


}


void HR_MONITR_SCREEN_TRANSITION(HealthmonitorMode desiredMode){
    switch (desiredMode) {  // Note: `desiredMode` is the variable being checked
        case HMM_BIOMONITOR:  // Colon, not semicolon
            break;
        case HMM_DAYHISTORY:
            break;
        case HMM_HISTORY:
            break;
        case HMM_SETTINGS:
            break;
        default:
            Serial.println("Unknown WatchMode!");
            WatchScreenUpdateInterval = 600;
            lockscreen_clock->updateContent("ERROR: Bad Mode");
            break;
    }
}

//HMM_BIOMONITOR, //Current fuckshit like that beep boop beeip in hopital
//HMM_DAYHISTORY,    //a bar graph over the past x days.
//HMM_HISTORY, //this month/historical trends. on long scales of time we'll just store average hr as waking/sleeping 
//HMM_SETTINGS

void HR_MONITR_SCREEN(void *pvParameters) { 
    (void)pvParameters;

    // Shared buffers for display
    char HM_buf[80];
    HealthmonitorMode currentMode = HMM_BIOMONITOR;  // Actual variable to switch on


//i hate every minute of this dogshit
    for (;;) {
        if (IsScreenOn && lockscreen_clock) {
            unsigned long now = millis();
            switch (currentMode) {  // Switching on a variable, not a type
                case HMM_BIOMONITOR:
                    // Handle biomonitor mode
                    break;
                case HMM_DAYHISTORY:
                    // Handle day history
                    break;
                case HMM_HISTORY:
                    // Handle long-term history
                    break;
                case HMM_SETTINGS:
                    // Handle settings
                    break;
                default:
                    Serial.println("Unknown hpmode!");
                    WatchScreenUpdateInterval = 600;
                    lockscreen_clock->updateContent("ERROR: Bad Mode");
                    break;
            }
        }
        clearScreenEveryXCalls(1000); //sometimes screen has weird update colisions, this resets it. sure it's spagetti and will make it stutter, but whatever man. temp only, do not use in prod. 
       vTaskDelay(pdMS_TO_TICKS(WatchScreenUpdateInterval));
    }//end for;;
}

// app nfc===================================================================================================================================================================================
std::shared_ptr<Window> nfc_app;
typedef enum{
NFC_OFF,
NFC_PLAYBACK,
NFC_RECORD,
}NFC_MODE;

    WindowCfg nfcStatusCfg = {
    10, 50,         // x, y
    100, 20,        // width, height
    false, false,   // auto-align, wrap text
    1,              // text size
    true,           // borderless
    tcol_secondary, tcol_background, tcol_primary,
    500             // update interval (ms)
};
/*
// Create during setup

void setupNFCUI() {
    nfcStatusWindow = std::make_shared<Window>("nfc_app", nfcStatusCfg, "NFC: OFF");
    windowManagerInstance->registerWindow(nfcStatusWindow);
}

static std::shared_ptr<Window> nfcStatusCfg;

void NFC_SCREEN_TRANSITION(NFC_MODE desiredMode) {
    // Update UI first
    switch(desiredMode) {
        case NFC_OFF:
            nfcStatusLabel->updateContent("NFC: rdy");
            break;
        case NFC_PLAYBACK:
            nfcStatusLabel->updateContent("NFC: PLAYBACK");
            //need to have a submenu for this? that lists em off the disk?
            break;
        case NFC_RECORD:
            nfcStatusLabel->updateContent("NFC: RECORDING");
            break;
    }
    
    // Then change hardware mode
    nfcManager.setMode(desiredMode);
}


void nfcTask(void *pvParameters) {
    NFCManager nfcManager(IRQ, RST);//should probably be in setup but
    nfcManager.begin(); 
    setupNFCUI();//we can't begin this in main for obvious reasons
    for (;;) {
        nfcManager.update();
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz update rate
    }
}
//app settings [dogshit gear icon,general settings]================================================================================================================================================
//=================================================================================================================================================================================================
typedef enum {
    GSLC_POWER,         // sleep modes
    GSLC_ALERTS,        // notifications, alarms
    GSLC_DISPLAY,       // screen settings
    GSLC_DATA,          // storage, sd card
    GSLC_WIRELESS,      // wifi, bt
    GSLC_EXT_HARDWARE,  // modules, sensors
    GSLC_CATEGORY_COUNT
} GlobalSettingsListCategory;

const char* GlobalSettingsListCategoryNames[] = {
    "power",        // sleep modes
    "alerts",       // notifications, alarms
    "display",      // screen settings
    "data",         // storage, sd card
    "wireless",     // wifi, bt
    "ext_hardware"  // modules, sensors
};

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
typedef struct {
    PowerSettings power;
    AlertsSettings alerts;
    DisplaySettings display;
    DataSettings data;
    WirelessSettings wireless;
    // ext_hardware can be fleshed out later
} GlobalSettings;

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
}*/


//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//infared remote=-----------=-=--==-=-=-=-=-=-=-===-=-=--=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef enum {
    IR_RECORD, //record witht hte on device ir sensor
    IR_IDLE, //do nothing
    IR_PLAYBACK,//remote controll emulation-ir blaster
    IR_FLASHLIGHT,//what do you think,genius
    IR_BEACON //flash with adjustable r8
}IR_REMOTE_ACTION;

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







//void loop() { } //i remember when merely this was how arduino code was written before freerots. we'd have to setup non blocking delay spagetti. 
// you could only do one thing at once that way, and it was funny. this was back when i wanted to make a "smart" watch when i was a teenager, but didn't fully-ass it like i'm doing right now
