//do not touch
#include "Wiring.h"

#define DO_ONCE(name)       \
    static bool _did_##name = false; \
    if (!_did_##name)       \
        for (_did_##name = true; _did_##name; _did_##name = false)
#define RESET_DO_ONCE(name) (_did_##name = false)


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
MAX30105 particleSensor;//particle sensor object

//imu
#include "FastIMU.h" 
#include "mdl_accelerometer.ino"
#define IMU_ADDRESS 0x68    //0x68 is the imu adress, if it needs to be fixed do that later
//#define PERFORM_CALIBRATION //Comment to disable startup calibration
MPU6500 IMU;               //Change to the name of any supported IMU! -extern so we can access this in accel module
calData calib = { 0 };  //Calibration data
AccelData accelData;     //Sensor data
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



//fuckj this were gonna put it wice
// Globals.cpp or at the top of your main .ino (outside setup and loop)
SPIClass spiBus(HSPI);
Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST);
bool deviceIsAwake=true;

//Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST); //note: &spiBus is required to pass main spi 


WindowManager* windowManagerInstance = nullptr;
static std::shared_ptr<Window> lockscreen_watchface;
bool IsScreenOn=true;
//add the encoders
#include "inputHandler.h"


int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;





void setup() {



    // Initialize hardware
    delay(148);
    Serial.begin(115200);
  delay(100);
    spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);


   // spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);


    screen_on();
    screen_startup();
    tft.fillScreen(BLACK);
    set_orientation(0);
    
SetupHardwareInput();//rotorary encoder things



if (!SD.begin(SPI_CS_SD, spiBus)) {
    Serial.println("[ERROR] SD.begin failed.");
    Serial.println("Check wiring, CS pin, card format, and card presence.");
    for (int i = 0; i < 3; ++i) {
      Serial.printf("Retrying SD init (%d/3)...\n", i + 1);
        delay(500);
        if (SD.begin(SPI_CS_SD)) {
            Serial.println("SD reinit successful.");
            break;
        }
    }

if (!SD.begin(SPI_CS_SD, spiBus)) {
        Serial.println("[FATAL] SD init failed");
        tft.fillScreen(BLACK);
        tft.setTextColor(RED);
        tft.setCursor(0, 0);
        tft.print("SD Error.\nCheck card.");
        // maybe blink LED or set a flag
        return; // gracefully exit setup
    }
}




    // Get window manager instance
    windowManagerInstance = WindowManager::getWinManagerInstance();
    if (!windowManagerInstance) {
        Serial.println("WindowManager failed to initialize");
        return;
    }



Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
 // —— SHOW ROOT ——
  File root = SD.open("/");
  Serial.println("Listing / ...");
  listFiles(root);
  root.close();

  // —— SHOW /img ——
  File imgdir = SD.open("/img");
  Serial.println("Listing /img ...");
  listFiles(imgdir);
  imgdir.close();

  // —— TEST hi.txt ——
  if (SD.exists("/docs/hi.txt")) {
    Serial.println("Found /docs/hi.txt:");
    File tf = SD.open("/docs/hi.txt");
    while (tf && tf.available()) {
      Serial.write(tf.read());
    }
    if (tf) tf.close();
    Serial.println("\n---");
  } else {
    Serial.println("/docs/hi.txt NOT found");
  }
File bmpFile = SD.open("/img/the_kitty.bmp");
if (!bmpFile) {
  Serial.println("Failed to open /img/the_kitty.bmp");
} else {
  Serial.println("Opened /img/the_kitty.bmp");

  // Read and dump first 54 bytes (typical BMP header size)
  Serial.println("BMP Header (54 bytes):");
  for (int i = 0; i < 54; i++) {
    if (!bmpFile.available()) break;
    uint8_t b = bmpFile.read();
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Optionally dump next 50 bytes of image data to check reading after header
  Serial.println("Next 50 bytes (image data):");
  for (int i = 0; i < 50; i++) {
    if (!bmpFile.available()) break;
    uint8_t b = bmpFile.read();
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  Serial.println();

  bmpFile.close();
}
//ready watch screen    

        WindowCfg cfg = {4, 64, 124, 32, false, false, 2, true, 0xFFFF, 0x0000, 0xFFFF, 1000};
        lockscreen_watchface = std::make_shared<Window>("lockscreen_watchface", cfg, "HH:MM:SS");
// xypos sizepos, auto align,wrap text, text size, borderless? , border background text color, update ms
        windowManagerInstance->registerWindow(lockscreen_watchface);

//myWindow->updateContent("<setcolor(0x99F02)>let's show <setcolor(0x0FF2)>you all <setcolor(0xE602)> what we can do<setcolor(0x0F2E)> :D WE CAN TYPE SO MUCH FUCKING BULLSHIT IT'S INSANE GRAAAAAAAAAAAAAAAAAA10947865091736450876108457620345AAAAAAAAAAAAAGGGHHHHH ");

//myWindow->SetBgColor(0xFFFF);
//delay(2200);
//myWindow->WinDraw(); 
//myWindow->updateContent("MEOW<setcolor(0x99F02)>MEOW MEOWMEOW"); 
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

//updateHRsensor();//heart rate monitor-best done every 10ms?
//PollEncoders(); //user input-100x/s but only when device is awake
//updateIMU();//poll gyro-2-10hz or so
//xTaskCreatePinnedToCore(SensorTask, "SensorTask", 4096, NULL, 1, NULL, 1);  // Sensor on core 1
xTaskCreatePinnedToCore(watchscreen, "watchscreen", 8192, NULL, 1, NULL, 0); // Watchscreen on core 0


}
/*
void SensorTask(void *pvParameters) {
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
                updateIMU();
                lastIMU = now;
            }
        } else {
            if (now - lastIMU >= imuIntervalSleep) {
                updateIMU();
                lastIMU = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(4));  // Give CPU some breathing room
    }
}*/

void watchscreen(void *pvParameters) {
    for (;;) {

        if (IsScreenOn && lockscreen_watchface) {
            std::string timeStr = 
                (currentHour   < 10 ? "0" : "") + std::to_string(currentHour)   + ":" +
                (currentMinute < 10 ? "0" : "") + std::to_string(currentMinute) + ":" +
                (currentSecond < 10 ? "0" : "") + std::to_string(currentSecond);

            lockscreen_watchface->updateContent(timeStr);
        }
        updateCurrentTimeVars();
        vTaskDelay(pdMS_TO_TICKS(1000)); // delay 1 second
    }
    
}






void loop() {
   
}