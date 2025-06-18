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
AccelData  imuAccel;      //Sensor data
GyroData gyroData;


//nfc-rfid
#include <Adafruit_PN532.h>
//#include <Adafruit_Sensor.h>


//wireles comunication
//#include <RadioLib.h>

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
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

int16_t temp_c=69;
extern int AVG_HR;

//fuckj this were gonna put it wice
// Globals.cpp or at the top of your main .ino (outside setup and loop)
SPIClass spiBus(HSPI);
Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST);
bool deviceIsAwake=true;

//Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST); //note: &spiBus is required to pass main spi 

//init windows
WindowManager* windowManagerInstance = nullptr;

static std::shared_ptr<Window> lockscreen_clock;
static std::shared_ptr<Window> lockscreen_biomon;
static std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;






//add the encoders
#include "inputHandler.h"


int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;
NormieTime CurrentNormieTime; //real current time


        

/*
        WindowCfg d_ls_sico_cfg = {0, 64, 128, 8, false, false, 1, true, 0xFFFF, 0x0000, 0xFFFF, 1000};
        lockscreen_systemIcons = std::make_shared<Window>("lockscreen_systemIcons", d_ls_sico_cfg, "");
            // xypos, sizexy, auto align,wrap text, text size, borderless? , border background text color, update ms
        windowManagerInstance->registerWindow(lockscreen_systemIcons);

*/


//Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

//updateHRsensor();//heart rate monitor-best done every 10ms?
//PollEncoders(); //user input-100x/s but only when device is awake
//updateIMU();//poll gyro-2-10hz or so
//xTaskCreatePinnedToCore(sensortick, "sensortick", 4096, NULL, 1, NULL, 1);  // Sensor on core 1
//xTaskCreatePinnedToCore(INPUT_tick, "INPUT_tick", 2048, NULL, 2, NULL, 1); 
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

    
WindowCfg d_ls_c_cfg = {0, 64, 127,42, false, false, 2, true, 0xFCCF, 0x0000, 0xFCCF, 1000};
        lockscreen_clock = std::make_shared<Window>("lockscreen_clock", d_ls_c_cfg, "HH:MM:SS");
    windowManagerInstance->registerWindow(lockscreen_clock);
    DBG_PRINTLN("Clock OK");



        WindowCfg d_ls_b_cfg = {
            110, 0, //x y pos
             20, 9, //width, height
              false, false,
               1, 
               true, 
               0xFFFF, 0x0000, 0xFFFF, 
               1000};

            lockscreen_biomon = std::make_shared<Window>("lockscreen_biomon", d_ls_b_cfg, "?bpm");
    windowManagerInstance->registerWindow(lockscreen_biomon);
    DBG_PRINTLN("Biomon OK");

TFillRect(0,0,128,128,0x0000);//black screen out


            WindowCfg d_ls_th_cfg = {  8,      // x
    8,      // y
    50,     // width
    10,     // height
    false,  // auto align?
    false,  // wrap text?
    1,      // text size
    false,  // borderless?
    0xFFFF, // border color
    0x0CC0, // background color
    0xFF00, // text color (green?)
    1000    // update ms
};

        lockscreen_thermometer = std::make_shared<Window>("lockscreen_thermometer", d_ls_th_cfg, "tmp");
    windowManagerInstance->registerWindow(lockscreen_thermometer);
    DBG_PRINTLN("Thermo OK");

    xTaskCreatePinnedToCore(watchscreen, "watchscreen", 12288, NULL, 1, NULL, 0);
    DBG_PRINTLN("watchscreen task OK");

    DBG_PRINTLN("SETUP DONE");
}

//ready watch screen    




 /* old dial poll code
void INPUT_tick(void *pvParameters) {
    const TickType_t hrInterval = pdMS_TO_TICKS(8);
    const TickType_t encoderInterval = pdMS_TO_TICKS(10);
    const TickType_t imuIntervalAwake = pdMS_TO_TICKS(100);
    const TickType_t imuIntervalSleep = pdMS_TO_TICKS(500);

    TickType_t lastHR = xTaskGetTickCount();
    TickType_t lastEncoder = lastHR;
    TickType_t lastIMU = lastHR;

    for (;;) {
        TickType_t now = xTaskGetTickCount();

        if (now - lastHR >= hrInterval) {
            updateHRsensor();
            lastHR = now;
        }

        if (deviceIsAwake) {
            if (now - lastEncoder >= encoderInterval) {
                PollEncoders();
                lastEncoder = now;
            }
            if (now - lastIMU >= imuIntervalAwake) {
                pollAccelAndUpdateBuffer(); isFacingUp(); 
                lastIMU = now;
            }
        } else {
            if (now - lastIMU >= imuIntervalSleep) {
                pollAccelAndUpdateBuffer(); isFacingUp(); //only runs in sleep mode, doesn't conflict with sensor tick
                lastIMU = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(4));  // Give CPU some breathing room
    }
}
void INPUT_tick(void *pvParameters) {
    for (;;) {        
            updateHRsensor();
            //PollEncoders();
            vTaskDelay(pdMS_TO_TICKS(5));  // Give CPU some breathing room
    }
}

void sensortick(void *pvParameters){ //polls sensors and buttons and stuff. 
if (deviceIsAwake) {
    for(;;){
        
    //temp_c = static_cast<int8_t>(roundf(IMU.getTemp())); //the temperature provided here is in no way accurate, replace with real sensor

    pollAccelAndUpdateBuffer(); isFacingUp(); //see if it's up or down, no shit. and update it. i need to run this at 10 hz or so but OH WELL MAN.
    vTaskDelay(pdMS_TO_TICKS(2000)); //decrease lamfo
    }
 }//end awake statement
}*/


void watchscreen(void *pvParameters) {
    for (;;) {


        if (IsScreenOn && lockscreen_clock) { 
            //clock
std::string timeStr = 
    (CurrentNormieTime.hour < 10 ? "0" : "") + std::to_string(CurrentNormieTime.hour) + ":" +           //hh
    (CurrentNormieTime.minute < 10 ? "0" : "") + std::to_string(CurrentNormieTime.minute) + ":" +       //mm
    (CurrentNormieTime.second < 10 ? "0" : "") + std::to_string(CurrentNormieTime.second) +             //ss
    "<n> <textsize(1)>  " //new line+small text size
    +TRIchar_month_names[CurrentNormieTime.month] + " " +std::to_string(CurrentNormieTime.day) ; //month/day

lockscreen_clock->updateContent(timeStr);

            lockscreen_clock->updateContent(timeStr);

            //temperature
            std::string ThermomSTR = std::to_string(temp_c)+"C";
            lockscreen_thermometer->updateContent(ThermomSTR);


            //heart rate lockscreen_biomon
            //shows hr/bo2 as int
            
          //  lockscreen_biomon->updateContent("hr" + std::to_string(AVG_HR)); //needs update called often

           // lockscreen_systemIcons
          // lockscreen_systemIcons->updateContent
        }
        updateCurrentTimeVars();


       
        vTaskDelay(pdMS_TO_TICKS(1000)); // delay 1 second




    }
    
}

//xTaskCreatePinnedToCore(watchscreen, "watchscreen", 12288, NULL, 1, NULL, 0); // Watchscreen on core 0




/*
void sensortick(void *pvParameters){ //polls sensors and buttons and stuff. 
    for(;;){
        
    temp_c = static_cast<int8_t>(roundf(IMU.getTemp())); //heads up no protection here but whatever. also i spent more time typing this than actually adding it but whatever run it at 1hz

    pollAccelAndUpdateBuffer(); isFacingUp(); //see if it's up or down, no shit. and update it. i need to run this at 10 hz or so but OH WELL MAN.
    vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
*/


void loop() {
   
}