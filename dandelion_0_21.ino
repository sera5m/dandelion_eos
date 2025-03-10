#include <Arduino.h>

//screen
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

//esp32 specifiic
#include "esp_pm.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp_sleep.h"
#include "esp_system.h" 

//freerots
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <mutex> 
#include "HardwareSerial.h" 
//arduino shi
#include <Wire.h>
#include <time.h>
#include <stdio.h>
#include <SPI.h>  // Include the SPI library
#include "SPI.h" //is this spi lib the same or a different one lol
#include <stdbool.h>

#define SPI_CLK_SPEED 56000000 // 56 MHz. very high! may increase power use
#define I2Cclock 400000 //set ic2 clock

// Pin definitions for spi
#define SCLK_PIN 18
#define MOSI_PIN 23 //may also be called poci on some devices
#define MISO_PIN  19 //may also be called pico 
#define DC_PIN   12
#define CS_PIN   13//also called ss

#define RST_PIN  14

//define pins for i2c 
#define SDA_PIN 27
#define SCL_PIN 26


#define INT_PIN 25  //d on the esp32


#include "mdl_mathHelper.ino" //add in my critical math helper functions that add vec3 and stuff

#include <memory>
#include <Preferences.h> // Library for handling NVS non volitile storage
Preferences preferences;

//wireless
#include <WiFi.h>
#include "esp_bt.h"
//#include "esp_bt_main.h"   // Required for esp_bluedroid functions
#include "esp_bt_device.h" // Optional  Bluetooth device functions



//heartrate
#include "MAX30105.h" //sparkfun lib
#include "heartRate.h"
#include "spo2_algorithm.h"//aw lawd
MAX30105 particleSensor;//particle sensor object

///////gyro imu thing
//#include "quaternionFilters.h"
#include "FastIMU.h" 
#include "mdl_accelerometer.ino"
#include "Madgwick.h"
#define IMU_ADDRESS 0x68    //0x68 is the imu adress, if it needs to be fixed do that later
//#define PERFORM_CALIBRATION //Comment to disable startup calibration
 MPU6500 IMU;               //Change to the name of any supported IMU! -extern so we can access this in accel module
calData calib = { 0 };  //Calibration data
 AccelData accelData;     //Sensor data
 GyroData gyroData;

// Define the extern variables


MagData IMUMag;
Madgwick filter;

//hardware.
//#include "FS.h" //doesn't work out here needs to be in storage module?????
#include <SD.h> //currently defaulting to the esp32 version. in other versions of the soft we may need to use a different lib

#include "fileSystem.ino"
#include <nvs_flash.h>
#include <nvs.h>

//important other libraries
#include <iostream>
#include <PCF8574.h>// Rob Tillaart's PCF8574 library, not adafruit!!
PCF8574 PCF_01(0x38); //yes it's this idk
//todo:declare pcf object

#include <Adafruit_Sensor.h>


//wireles comunication
#include <RadioLib.h>



// Include basic functions
#include "module_math_render_base.ino" //handles screens
#include "lillypad_renderer.ino" //handles rendering
#include "mdl_pwr_mngr.ino"
#include "mdl_timeKeeping.ino"  //the clock is ticking
#include "mdl_diagnosticTools.ino"
#include "labyrinth.ino" //apps


//more shit
#include <cstdlib>  // For rand()
#include <ctime>    // For seeding rand()

/*
Uncomment and set up if you want to use custom pins for the SPI communication
#define REASSIGN_PINS
int sck = -1;
int miso = -1;
int mosi = -1;
int cs = -1;
*/



//math functions misc i guess
#include <vector>
#include <stdint.h> // For int8_t
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define megahertz 1000000 //because it's unclear seeing like 6 zeroes elsewhere
 int currentHour = 0;
 int currentMinute = 0;
 int currentSecond = 0;
 float temperature = 69;


#include <iomanip> // For output precision
#include <chrono>  // For high-precision timing


//less important stuff 
unsigned long previousMillis = 0; // Define and initialize
unsigned long frameTime = 0;      // Define and initialize


//declare the task functions for multicore programs
void taskDrawScreen(void *pvParameters); //render the screen using known variables
void taskUpdateSensors(void *pvParameters); //update the sensors 
void taskUpdateHeartRate(void *pvParameters);









// Setup!!!
void setup() {

  
srand(time(NULL)); // Seed RNG

//config wifi and bluetooth to be off
  WiFi.mode(WIFI_OFF);  // Turn off Wi-Fi
  WiFi.disconnect(true); // Ensure it stays off and disconnected
  //  esp_bluedroid_disable(); // Disable Bluetooth stack
 // esp_bluedroid_deinit();  // De-initialize Bluetooth stack
  btStop();                // Turn off Bluetooth
  

  // Initialize SPI
 
  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));  // 40 MHz SPI clock speed
SPI.endTransaction();


 Serial.begin(115200); 
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with correct pins
  // Start display
 screen_on();  // Call to initialize the screen with proper SPI settings


pcf.begin(); //init i/o expander

// Set P0–P5 as inputs with pull-up resistors
for (int i = 0; i < 6; i++) {
    pcf.pinMode(i, INPUT_PULLUP);
}

// Attach interrupt to INT pin
pinMode(INT_PIN, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(INT_PIN), handleInterrupt, FALLING);

//note: don't add polling, this is interrupt based


  delay(1);
  //Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C
  screen_on();
  screen_startup(); // Start the screen up
  tft.fillScreen(BLACK); // Erase it
  set_orientation(0);

  // Print debug devices identified to this watch with a status as OK or not
  tft.setTextSize(1); 
  tft.setTextColor(PEACH);
tft.fillScreen(BLACK);
  // void deviceCheck();
  tft.setCursor(0, 0); 
   tft.setTextSize(1); 
  tft.printf("DANDELION OS");
    tft.setCursor(0, 64);
      tft.setTextSize(1); 
  tft.printf("ATERNUM TECHNOLOGIES");


    // Initialize RTC with stored or default time
    initializeRTC();
  
    // Print the current local time
    //todo:renable this with correct current args


//initialize stupid sensors here
HRsensorSetup();
/*
//start the imu
 if (IMU.init({true}, 0x68) != 0) { // Use the 6500 initialization process  i hate this, when prototyping i paid for a 9 series
    Serial.println("IMU initialization failed!");
    while (1);
}
*/
// Verify the devices are connected
delay(20);
tft.fillScreen(BLACK);

tft.setTextColor(WHITE);
scanI2C(); // Scan the I2C bus to verify if they're connected
delay(1);
scanSPI();
delay(800); // Let the user read things.
tft.fillScreen(BLACK);

// Now see if the SD card is connected on the SPI
if (!SD.begin()) { // Check if the SD card initialization is successful
    Serial.println("Card Mount Failed");
    tft.printf("SD CARD MOUNT FAILED, CHECK if it's in");
} else {
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        tft.printf("No SD card attached\n");
    } else {
        Serial.print("SD Card Type: ");
        if (cardType == CARD_MMC) {
            Serial.println("MMC");
        } else if (cardType == CARD_SD) {
            Serial.println("SDSC");
        } else if (cardType == CARD_SDHC) {
            Serial.println("SDHC");
        } else {
            Serial.println("UNKNOWN");
        }

        // Start up the storage drive if connected
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        tft.printf("microsd storage: %lluMB\n", SD.totalBytes() / (1024 * 1024));
        tft.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    }
}

//preboot phase done, move to standard tasks

 // Create task for drawing to the screen (Core 0)
  xTaskCreatePinnedToCore(taskDrawScreen,"Draw Screen",10000,NULL,1,NULL,0);

  // Create task for updating sensors and clock (Core 1)
  xTaskCreatePinnedToCore(
    taskUpdateSensors, // Function to be called (updating sensors)
    "Update Sensors",  // Name of the task (for debugging)
    10000,             // Stack size (words)
    NULL,              // Parameters (none)
    1,                 // Priority (1)
    NULL,              // Task handle (none)
    1                  // Run on Core 1
  );





xTaskCreatePinnedToCore(taskUpdateHeartRate,"HRSENSOR",2048,NULL,1,NULL,1);
//xTaskCreatePinnedToCore(taskPmeterUpdate,"pmeter",8192,NULL,1,NULL,1);
//setup ends here


}


void loop() {
  //nothing for now
}

//you put updates here


//update_IMU(); //update motion sensor and it's samples, CALL EVERY 50ms