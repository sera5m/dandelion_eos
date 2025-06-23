//do not touch
#include "Wiring.h" //my hardware definitions

//esp32-s3 hardware
#include "USB.h"
#include "USBCDC.h"
#include <Wire.h>


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


#include "watch_Settings.h" //configuration file for settings

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
MAX30105 particleSensor;//particle sensor object
   
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




//include my own stuff
 //#include "AT_SSD1351.ino"
#include "Micro2D_A.ino"  // The library

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

int16_t temp_c=69;
extern int AVG_HR;

//fuckj this were gonna put it wice
// Globals.cpp or at the top of your main .ino (outside setup and loop)


Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST);
bool deviceIsAwake=true;

//Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST); //note: &spiBus is required to pass main spi 

//init windows
WindowManager* windowManagerInstance = nullptr;

static std::shared_ptr<Window> lockscreen_clock; QueueHandle_t lockscreenQueue = nullptr;
static std::shared_ptr<Window> lockscreen_biomon;
static std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;


#include <atomic>

enum WatchMode {
    WM_MAIN, //THE general lock screen
    //other modes
    WM_STOPWATCH, 
    WM_ALARMS, //set your alarms, they'll automatically run
    WM_TIMER,

    //settings
    WM_NTP_SYNCH, 
    WM_SET_TIME,
    WM_SET_TIMEZONE
};
uint16_t tcol_primary=0x07E0;
uint16_t tcol_secondary=0x0000;
uint16_t tcol_tertiary=0x4208;
uint16_t tcol_highlight=0xF805;
uint16_t tcol_background=0x29e6;
//var references to set for the theme

list_Themes Current_Theme=mint; //set the current theme to a nice default

SetDeviceTheme(mint);

WindowCfg d_ls_c_cfg = { //clock
    14, 64, //xy
    100, 42, //wh
    false, false, //auto align,wraptext
    2, //text size
    true,//borderless?
    &tcol_secondary, &tcol_background, &tcol_primary, // <-- pass addresses!. colors
    1000 //update interval ms
};

WindowCfg d_ls_b_cfg = {//heart monitor
    86, 0,
    50, 12,
    false, false,
    1,
    true,
    &tcol_secondary, &tcol_background, &tcol_primary,
    1000
};

WindowCfg d_ls_th_cfg = {//thermometer
    8, 0,
    50, 12,
    false, false,
    1,
    false,
    &tcol_secondary, &tcol_background, &tcol_primary,
    1000
};




extern QueueHandle_t processInputQueue;

std::atomic<WatchMode> currentWatchMode = WM_MAIN;
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


void setup() {

//set up structs and configs
   ON_BOOT_VISUAL_CONFIG(); 


//end setup struct+cfg
    delay(148); 
    Serial.begin(115200);
    

  delay(100); // Let this Bitch™ boot
SetDeviceTheme(mint); //apply and boot

    _dbg_ypos = 0; // Reset debug print position
    spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    screen_on();
    screen_startup();
    tft.fillScreen(tcol_background);
    set_orientation(0);

    DBG_PRINTLN("BOOT BEGIN");

Wire.begin(SDA_PIN, SCL_PIN);

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


        lockscreen_clock = std::make_shared<Window>("lockscreen_clock", d_ls_c_cfg, "HH:MM:SS");
    windowManagerInstance->registerWindow(lockscreen_clock);
    DBG_PRINTLN("Clock OK");

            lockscreen_biomon = std::make_shared<Window>("lockscreen_biomon", d_ls_b_cfg, "XXXbpm");
    windowManagerInstance->registerWindow(lockscreen_biomon);
    DBG_PRINTLN("Biomon OK");

        lockscreen_thermometer = std::make_shared<Window>("lockscreen_thermometer", d_ls_th_cfg, "XXXC");
    windowManagerInstance->registerWindow(lockscreen_thermometer);


   // DBG_PRINTLN("Thermo OK");

TFillRect(0,0,128,128,0x0000);//black screen out






processInputQueue = xQueueCreate(8, sizeof(S_UserInput)); //set up default que
xTaskCreate(watchscreen, "WatchScreen", 4096, NULL, 1, NULL);//core 0 watch screen 


    //DBG_PRINTLN("watchscreen task OK");
    xTaskCreatePinnedToCore(INPUT_tick, "INPUT_tick", 2048, NULL, 2, NULL, 1); //core 1 sensor updates


//evil spagetti
//processInputQueue = lockscreenQueue;//2. tell input router to use that que
currentinputTarget = R_toProc; //3. MANUALLY alter input handling values to route to proscesses. we



    DBG_PRINTLN("SETUP DONE");
    delay(100);

}//end void setup

    int WatchScreenUpdateInterval=500;

void WatchScreenTransition(WatchMode newmode){
                switch (newmode){



                case WM_MAIN:
                WatchScreenUpdateInterval=500;
                //update bg?
                lockscreen_clock->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height); lockscreen_clock->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y); //reset to original config size reguardless of original config
                    break;

                case WM_STOPWATCH: {
                    WatchScreenUpdateInterval=100;//update WAY more frequently at 100ms
                    lockscreen_clock->ResizeWindow(d_ls_c_cfg.width+14, d_ls_c_cfg.height);//expand for more digits, .xyz expand by 14 pixels
                     lockscreen_clock->MoveWindow(d_ls_c_cfg.x-14, d_ls_c_cfg.y);//move 14 pixels to the left to offset more digits.
                                                        //x now has effective increased size of 28px. probably better if i did this as a new struct but who cares. 
                    break;
                }

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
            }//end switch

}//end fn






int stopwatchElapsed=0;

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
                    WatchScreenTransition(WM_STOPWATCH);
                    break;
                case key_right:
                case key_back:
                    currentWatchMode = WM_MAIN;
                    WatchScreenTransition(WM_MAIN);
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
    while (xQueueReceive(processInputQueue, &junk, 0) == pdPASS) {}
    inputCount = 0;
}

        updateHRsensor();
        PollEncoders();
        PollButtons();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
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

            switch (currentWatchMode.load()) {

                case WM_MAIN:
                WatchScreenUpdateInterval=500;
                    snprintf(buf, sizeof(buf), "%02d:%02d:%02d<n> %s %d",
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
                    WatchScreenUpdateInterval=100;//update WAY more frequently at 100ms
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

                    snprintf(buf, sizeof(buf), "%02u:%02u:%02u.%03u %s", h, m, s, ms,
                                             stopwatchRunning ? "RUN" : "STOP");


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

        updateCurrentTimeVars();
        vTaskDelay(pdMS_TO_TICKS(WatchScreenUpdateInterval));
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



void loop() { } //i remember when merely this was how arduino code was written before freerots. we'd have to setup non blocking delay spagetti.  you could only do one thing at once that way, and it was funny. this was back when i wanted to make a "smart" watch when i was a teenager, but didn't fully-ass it like i'm doing right now