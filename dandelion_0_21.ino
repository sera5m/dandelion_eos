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


#include "kernel.ino" //add in my critical math helper functions that add vec3 and stuff

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


void loop() {
  //nothing for now
}



//todo: stop feellng sad