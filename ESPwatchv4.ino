//do not touch
#include "Wiring.h"






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
#include <SPI.h>


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
#include <RadioLib.h>


//storate
#include "SDFS.ino"
#include <SD.h> //esp specific lib
#include <nvs_flash.h>
#include <nvs.h>






//include my own stuff
 //#include "AT_SSD1351.ino"
#include "Micro2D_A.ino"  // The library

#include "the_cat.h"//kitty test image, 128 px 16x 565 color bits




SPIClass spiBus(HSPI); // or VSPI (SPI3)
WindowManager* windowManagerInstance = nullptr;
Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC, OLED_RST); //note: &spiBus is required to pass main spi 




void setup() {
    // Initialize hardware
    Serial.begin(115200);

    spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    screen_on();
    screen_startup();
    tft.fillScreen(BLACK);
    set_orientation(0);




    // Get window manager instance
    windowManagerInstance = WindowManager::getWinManagerInstance();
    if (!windowManagerInstance) {
        Serial.println("WindowManager failed to initialize");
        return;
    }

    // Create window configuration
    WindowCfg winCfg = {
        0, 0,         // x, y
        126, 126,     // width, height
        false,        // AutoAlignment
        true,         // WrapText
        1,            // TextSize
        true,        // borderless
        0xFFAA,       // BorderColor 
        0x1234,       // bgColor (black)
        0xF000,       // TextColor
        100           // UpdateTickRate
    };

    auto myWindow = std::make_shared<Window>("MainWin", winCfg, "MEOWL???");
    windowManagerInstance->registerWindow(myWindow);

    // Canvas setup
    CanvasCfg canvasCfg = {
        10, 10,        // x, y (relative to window)
        80, 80,        // width, height
        true,         // borderless
        false,         // DrawBG
        0x0480,        // bgColor (green)
        0xFAF0,        // BorderColor
        myWindow.get() // parent window
    };

    // Create shared canvas instance
    //auto myCanvas = std::make_shared<Canvas>(canvasCfg, myWindow);

    // Add to window manually
    //myWindow->canvases.push_back(myCanvas); 


myWindow->WinDraw();
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());


/*
for (int iterator = 1; iterator <= 16; iterator++) {
    //myCanvas->clear();  // Clear previous circles
    myCanvas->AddCircle(64, 64, iterator * 4, randomColor(), 1);  
    myCanvas->CanvasUpdate(true);  // Smarter update only calling canvas :3
    //delay(2);  // More reasonable delay
}

delay(28);
myCanvas->ClearAll();
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
myWindow->updateContent("-_<");
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
myWindow->updateContent(">_<");
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
myWindow->updateContent(">_-");
*/
delay(2200);
//myWindow->updateContent("<setcolor(0x99F02)>let's show <setcolor(0x0FF2)>you all <setcolor(0xE602)> what we can do<setcolor(0x0F2E)> :D WE CAN TYPE SO MUCH FUCKING BULLSHIT IT'S INSANE GRAAAAAAAAAAAAAAAAAA10947865091736450876108457620345AAAAAAAAAAAAAGGGHHHHH ");

//myWindow->SetBgColor(0xFFFF);
//delay(2200);
//myWindow->WinDraw(); 
//myWindow->updateContent("MEOW<setcolor(0x99F02)>MEOW MEOWMEOW"); 
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
/*
myWindow->updateContent("i have spent<n>the last several weeks <n> creating this library<n>to add a window system to the esp32.<n><n><setcolor(0xEEEE)>this wasn't easy...");
delay(2280);
myWindow->updateContent("<setcolor(0xFFFF)> windows support live <setcolor(0x07E0)>recongifuration <setcolor(0xFFFF)>of their properties!");
myWindow->ForceBorderState(true);
myWindow->SetBorderColor(0x1FFF);
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  //myWindow->updateContent("Free heap: %d\n", ESP.getFreeHeap());
  
myWindow->WinDraw();
myWindow->updateContent("such as changing colors....");
delay(1234);
myWindow->SetBgColor(0x1234);
myWindow->WinDraw();
myWindow->updateContent("<setcolor(0x07E0)>or even size");
delay(1234);
*/
drawBitmapFromProgmem(the_cat, 0, 0, 128, 128);

}

void loop() {
   
}