#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

#include "esp_pm.h"
#include "esp_wifi.h"
#include "esp_bt.h"
#include "esp_sleep.h"
#include "esp_system.h" // For device ID
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "HardwareSerial.h" 
#include <Wire.h>
#include <time.h>
#include <stdio.h>
#include <SPI.h>  // Include the SPI library

#define SPI_CLK_SPEED 56000000 // 56 MHz. very high! may increase power use
#define I2Cclock 400000 //set ic2 clock

// Pin definitions for spi
#define SCLK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN  19 
#define DC_PIN   12
#define CS_PIN   13
#define RST_PIN  14

//define pins for scl. 
#define SDA_PIN 27
#define SCL_PIN 26

#include "mdl_mathHelper.ino" //add in my critical math helper functions that add vec3 and stuff
//#include "asm_helpies.s"

#include <Preferences.h> // Library for handling NVS non volitile storage
Preferences preferences;

//wireless
#include <WiFi.h>
#include "esp_bt.h"
//#include "esp_bt_main.h"   // Required for esp_bluedroid functions
#include "esp_bt_device.h" // Optional if you need Bluetooth device functions



//heartrate
#include "MAX30105.h"
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







#include <Adafruit_Sensor.h>


//wireles comunication
#include <RadioLib.h>



// Include basic functions
#include "module_math_render_base.ino"
#include "module_window_handler.ino"
#include "mdl_pwr_mngr.ino"
#include "mdl_timeKeeping.ino"  //the clock is ticking
#include "mdl_diagnosticTools.ino"




//hardware.
//#include "FS.h" //doesn't work out here needs to be in storage module?????
#include <SD.h>
#include "SPI.h"
#include "fileSystem.ino"
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
 int currentHour = 0;
 int currentMinute = 0;
 int currentSecond = 0;
 float temperature = 69;





//less important stuff 
unsigned long previousMillis = 0; // Define and initialize
unsigned long frameTime = 0;      // Define and initialize


//declare the task functions for multicore programs
void taskDrawScreen(void *pvParameters); //render the screen using known variables
void taskUpdateSensors(void *pvParameters); //update the sensors 
void taskUpdateHeartRate(void *pvParameters);









// Setup!!!
void setup() {

  //have the stupid math shit working, in the motion sensor
   initializeSampleCache();

//config wifi and bluetooth to be off
  WiFi.mode(WIFI_OFF);  // Turn off Wi-Fi
  WiFi.disconnect(true); // Ensure it stays off and disconnected
  //  esp_bluedroid_disable(); // Disable Bluetooth stack
 // esp_bluedroid_deinit();  // De-initialize Bluetooth stack
  btStop();                // Turn off Bluetooth
  

  // Initialize SPI
  SPI.begin();
  SPI.begin(SCLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));  // 40 MHz SPI clock speed
SPI.endTransaction();


 Serial.begin(115200); 
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with correct pins
  // Start display
 screen_on();  // Call to initialize the screen with proper SPI settings






  delay(1);
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C
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
    printLocalTime();


//initialize stupid sensors here
HRsensorSetup();

//start the imu
 if (IMU.init({true}, 0x68) != 0) { // Use the 6500 initialization process  i hate this, when prototyping i paid for a 9 series
    Serial.println("IMU initialization failed!");
    while (1);
}

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

//pedometer turned off because it's so laggy it's horrid
/*
void taskPmeterUpdate(void *pvParameters) {
  (void)pvParameters;  // Avoid unused parameter warning

  while (true) {
    // Update sensor data
    //    update_IMU();
   // int steps = countSteps();
   Serial.println("vector magnitude");
Vector3 v(3.0f, 4.0f, 0.0f);  // This should result in a magnitude of 5.0
Serial.print("x: "); Serial.println(v.x);
Serial.print("y: "); Serial.println(v.y);
Serial.print("z: "); Serial.println(v.z);
float magnitude = v.magnitude();
Serial.print("Magnitude: "); Serial.println(magnitude);

// Should print )
    // Print step count
   // Serial.print("Steps detected: ");
    //Serial.println(steps);

Serial.print("Free heap: ");
Serial.println(esp_get_free_heap_size());

  
    vTaskDelay(200 / portTICK_PERIOD_MS);  // Delay for n ms before next update.
  }
}
*/

// Task for drawing to the screen (Core 0)
void taskDrawScreen(void *pvParameters) {
  (void)pvParameters; // Avoid unused parameter warning

  while (true) {
 
updateLockScreen(); //update the lock screen,but window hander module does it 
    vTaskDelay(750 / portTICK_PERIOD_MS);  // Delay for 100ms before next update
    
  }
}

// Task for updating sensors and clock (Core 1)
void taskUpdateSensors(void *pvParameters) {
  (void)pvParameters; // Avoid unused parameter warning

  while (true) {
    // Update sensor data
   
   //replacement of temperature sensor with the mpu-s temperature reader
       temperature=IMU.getTemp(); //USING THE fstimu lib, get device temp. replaces tmp 102 sensor to save a few bucks
 //i should print this to screen to get a debug,honestly
 Serial.println(IMU.getTemp());
 
    // Update time
    updateStoredTime();  // Function to update time (currentHour, currentMinute, currentSecond)
   //updateheartrate();//commented out, this sensor needs to run faster
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay for 500ms before next update


//void printIMUdata();
    
 
  }
}

// Task for updating heart rate
void taskUpdateHeartRate(void *pvParameters) {
  (void)pvParameters;  // Avoid unused parameter warning

  while (true) {
    // Update sensor data
    updateheartrate(); // Make sure this function is defined elsewhere
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay for 10ms before next update. this is stupidly fast
  }
}

//update_IMU(); //update motion sensor and it's samples, CALL EVERY 50ms