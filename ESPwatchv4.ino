//do not touch
#include "Wiring.h"

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


#include "watch_Settings.h"

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





//nfc-rfid
#include <Adafruit_PN532.h>
//#include <Adafruit_Sensor.h>
Adafruit_PN532 nfc(HSPI, SPI_CS_NFC); // Create an instance of the PN532 using hardware SPI

//wireles comunication
//#include <RadioLib.h>



//add the encoders
#include "inputHandler.h"


int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;
NormieTime CurrentNormieTime; //real current time


QueueHandle_t processInputQueue; //absolutely needs to be here because freerots. hadndles proscess input        


//Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

//updateHRsensor();//heart rate monitor-best done every 10ms?
//PollEncoders(); //user input-100x/s but only when device is awake
//updateIMU();//poll gyro-2-10hz or so
//xTaskCreatePinnedToCore(sensortick, "sensortick", 4096, NULL, 1, NULL, 1);  // Sensor on core 1

//xTaskCreatePinnedToCore(watchscreen, "watchscreen", 12288, NULL, 1, NULL, 0); // Watchscreen on core 0
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
    delay(148);
    _dbg_ypos = 0; // Reset debug print position
    spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    screen_on();
    screen_startup();
    tft.fillScreen(BLACK);
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

    
WindowCfg d_ls_c_cfg = { //lock screen clock
    14, 64,
 100,42,//with, height
  false, false, 
  2,//text size
true,//border
0xFCCF, 0x0000, 0xFCCF, 
1000};//clock config


     WindowCfg d_ls_b_cfg = {//lock screen heart rate
     86, 0, //x y pos
     50, 12, //width, height
     false, false, //auto align,wrap
     1, 
     true, //borderless?
     0xFFFF, 0x0000, 0xFFFF, //border,background,text
    1000};

     WindowCfg d_ls_th_cfg = { //thermometer
    8,0, //y
    50,12,     // width,height
    false,  // auto align?
    false,  // wrap text?
    1,      // text size
    false,  // borderless?
    0xFFFF, // border color
    0x0CC0, // background color
    0xFFFF, // text color
    1000    // update ms
};



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






lockscreenQueue = xQueueCreate(8, sizeof(S_UserInput));
xTaskCreate(watchscreen, "WatchScreen", 4096, (void *)lockscreenQueue, 1, NULL);//core 0 watch screen and INPUT SCREEN QUE


    //DBG_PRINTLN("watchscreen task OK");
    xTaskCreatePinnedToCore(INPUT_tick, "INPUT_tick", 2048, NULL, 2, NULL, 1); //core 1 sensor updates




    DBG_PRINTLN("SETUP DONE");
    //delay(500);
}

void INPUT_tick(void *pvParameters) {
    for (;;) {        
            updateHRsensor();
            PollEncoders(); //may god help us all
            vTaskDelay(pdMS_TO_TICKS(5));  // Give CPU some breathing room
    }
}

void watchscreen(void *pvParameters) {
    QueueHandle_t inputQueue = static_cast<QueueHandle_t>(pvParameters);
    S_UserInput uinput;

    for (;;) {
        // Handle inputs
        if (xQueueReceive(inputQueue, &uinput, 0)) {
            // Example: React to input (exit on BACK key)
            if (uinput.key == keyback && uinput.isDown) {
                // Add your own handler logic
                DBG_PRINTLN("Back key pressed on lockscreen.");
            }
        }

        // Refresh screen display if active
        if (IsScreenOn && lockscreen_clock) {
            char timeStr[40];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d<n> <textsize(1)> %s %d",
                     CurrentNormieTime.hour,
                     CurrentNormieTime.minute,
                     CurrentNormieTime.second,
                     TRIchar_month_names[CurrentNormieTime.month],
                     CurrentNormieTime.day);
            lockscreen_clock->updateContent(timeStr);

            char thermoStr[4];
            snprintf(thermoStr, sizeof(thermoStr), "%dC", temp_c);
            lockscreen_thermometer->updateContent(thermoStr);

            char hrStr[6];
            snprintf(hrStr, sizeof(hrStr), "%dbpm", AVG_HR);
            lockscreen_biomon->updateContent(hrStr);
        }

        updateCurrentTimeVars();
        vTaskDelay(pdMS_TO_TICKS(1000));
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



void loop() {
   
}