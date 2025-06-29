//do not touch
//_Static_assert(Sizeof(oledcfg.clock_speed_hz)==4,"oopsies, clockspeedhz isn't an int");
#include "Wiring.h" //my hardware definitions

//esp32-s3 hardware
#include "USB.h"
#include "USBCDC.h"
#include <Wire.h>
#include "esp_heap_caps.h"
#include "C:\Users\dev\AppData\Local\Arduino15\packages\esp32\tools\esp32-arduino-libs\idf-release_v5.4-2f7dcd86-v1\esp32s3\include\esp_driver_spi\include\driver\spi_master.h" //i do not take chances

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

#include <time.h>
#include <stdio.h>


//time
#include "mdl_clock.h"

//storate
#include "SDFS.ino"
#include <SD.h> //esp specific lib
#include <nvs_flash.h>
#include <nvs.h>
#include <pgmspace.h>


/*
#define DBG_PRINTLN(x)  do { \
    tft.setCursor(0, _dbg_ypos); \
    tft.setTextColor(WHITE); \
    tft.setTextSize(1); \
    tft.println(x); \
    _dbg_ypos += 10; \                               
} while(0)*/

//int _dbg_ypos = 0;  // screen Y cursor


//#include "watch_Settings.h" //configuration file for settings
spi_device_handle_t oledSpiHandle;
spi_device_handle_t sdSpiHandle;
spi_device_handle_t nfcSpiHandle;
//note otta put the fucking 
//get fuckleries going
#define SPANBUF_SIZE 64
#include "esp_heap_caps.h"

if (!esp_spiram_is_initialized()) {
    printf("PSRAM not initialized!\n");
    return;
    //stp[ jere]
}

size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
printf("Free PSRAM: %d bytes\n", free_psram);
size_t buffer_size = 16 * 128*128; // screen width*ht*colors
void* vram_buffer = heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
if (vram_buffer == NULL) {
    printf("Failed to allocate PSRAM buffer!\n");
} else {
    printf("Allocated %d bytes from PSRAM\n", buffer_size);
}


// === Arduino SPI handle ===
//#include <SPI.h>
//#include <Adafruit_PN532.h>
//SPIClass myHSPI(HSPI);
//Adafruit_PN532 nfc(myHSPI, SPI_CS_NFC/*SPI_SCK,SPI_MISO,SPI_MOSI,SPI_CS_NFC*/); //unsure if i need to pass hspi because the example showed use of only a chip sel pin


// === ESP-IDF SPI device handles ===
spi_device_handle_t oled_handle;
spi_device_handle_t sd_handle;




//include my own stuff
 //#include "AT_SSD1351.ino"
#include "Micro2D_A.ino"  // The library

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>



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
MPU6500 IMU;               //Change to the name of any supported IMU! 
calData calib = { 0 };  //Calibration data
AccelData imuAccel;      //Sensor data
GyroData gyroData;

int16_t temp_c=69;
extern int AVG_HR;

//fuckj this were gonna put it wice
// Globals.cpp or at the top of your main .ino (outside setup and loop)

//default colors for face
//tpallt means theme palette
uint16_t tcol_primary = 0x07E0;
uint16_t tcol_secondary = 0xCCCC;
uint16_t tcol_tertiary = 0x4208;
uint16_t tcol_highlight = 0xF805;
uint16_t tcol_background = 0x29D7;

//“dummy” object for GFX primitives


typedef struct{
    uint16_t primary;
    uint16_t secondary; 
    uint16_t tertiary;  
    uint16_t highlight;
    uint16_t background;
}ui_color_palette;
/*

enum list_Themes{mint,hacker,specOps,terminal,userCustom};
list_Themes Current_Theme=mint; //set the current theme to a nice default




void SetDeviceTheme(list_Themes theme) {
    // Store the theme in the global so you know what the current one is
    Current_Theme = theme;

    const ui_color_palette* selectedTheme = nullptr;

    switch (theme) {
        case mint:
            selectedTheme = &UITHM_mint;
            break;
        case hacker:
            selectedTheme = &UITHM_hacker;
            break;
        case specOps:
            selectedTheme = &UITHM_specOps;
            break;
        case terminal:
            selectedTheme = &UITHM_terminal;
            break;
        case userCustom:
            selectedTheme = &UITHM_userCustom;
            break;
        default:
            Serial.println("Unknown theme enum. Defaulting to mint.");
            selectedTheme = &UITHM_mint;
            break;
    }

    // Null check in case something goes off the rails
    if (!selectedTheme) {
        Serial.println("SetDeviceTheme: selectedTheme is null");
        return;
    }

    // Apply palette safely
    tcol_primary    = selectedTheme->primary;
    tcol_secondary  = selectedTheme->secondary;
    tcol_tertiary   = selectedTheme->tertiary;
    tcol_highlight  = selectedTheme->highlight;
    tcol_background = selectedTheme->background;
}

// You should also wrap the ApplyThemeAllWindows call to prevent invalid color usage
void ApplyCurrentThemeToUI() {
    // Always apply AFTER SetDeviceTheme
    if (WinManagerInstance) {
        WinManagerInstance->ApplyThemeAllWindows(tcol_secondary, tcol_background, tcol_primary);
    }
}*/

//Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC_DIRECT_REF, OLED_RST);
//MySSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, -1, OLED_DC_DIRECT_REF, OLED_RST);
bool deviceIsAwake=true;

//Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &spiBus, SPI_CS_OLED, OLED_DC_DIRECT_REF, OLED_RST); //note: &spiBus is required to pass main spi 

//init windows
WindowManager* windowManagerInstance = nullptr;

static std::shared_ptr<Window> lockscreen_clock; QueueHandle_t lockscreenQueue = nullptr;
static std::shared_ptr<Window> lockscreen_biomon;
static std::shared_ptr<Window> lockscreen_thermometer;
//static std::shared_ptr<Window> lockscreen_systemIcons;

bool IsScreenOn=true;


typedef enum{
    WM_MAIN, //THE general lock screen
    WM_STOPWATCH, 
    WM_ALARMS, //set your alarms, they'll automatically run
    WM_TIMER,
    WM_NTP_SYNCH, //settings
    WM_SET_TIME,
    WM_SET_TIMEZONE
}WatchMode;

volatile WatchMode currentWatchMode = WM_MAIN;



WindowCfg d_ls_c_cfg = { //clock
    14, 64, //xy
    100, 42, //wh
    false, false, //auto align,wraptext
    2, //text size
    true,//borderless?
    0xCCCC, 0x29e6, 0x07E0, 
    1000 //update interval ms
};

WindowCfg d_ls_b_cfg = {//heart monitor
    86, 0,
    50, 12,
    false, false,
    1,
    true,
    0xCCCC, 0x29e6, 0x07E0,
    1000
};

WindowCfg d_ls_th_cfg = {//thermometer
    8, 0,
    50, 12,
    false, false,
    1,
    false,
    0xCCCC, 0x29e6, 0x07E0,
    1000
};




 QueueHandle_t processInputQueue;


bool stopwatchRunning = false;
unsigned long stopwatchStart = 0;


//nfc-rfid

//#include <Adafruit_Sensor.h>

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





void scanI2C() {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning I2C bus...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x"); //DBG_PRINTLN("ok");
      Serial.println(address, HEX); //DBG_PRINTLN(String(address, HEX));
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at 0x");// DBG_PRINTLN("err at");
      Serial.println(address, HEX); //DBG_PRINTLN(String(address, HEX));
    }
  }

  if (nDevices == 0){
    Serial.println("No I2C devices found\n"); //DBG_PRINTLN("0I2c devices"); 
    } else{
    Serial.println("I2C scan complete\n"); //DBG_PRINTLN("I2C done");
    }

}

int WatchScreenUpdateInterval=500;

//please for the love of christ use c++20 designated initializers to avoid old 8 bit c header legacy dogshit or the arduino ide preproscessor will eat your fucking lunch
// and try narrowing this EVEN THOUGH the struct itself for this in spi_mater.h 
//located at esp32-s3 C:\Users\user\AppData\Local\Arduino15\packages\esp32\tools\esp32-arduino-libs\idf-release_v5.4-2f7dcd86-v1\esp32s3\include\esp_driver_spi\include\driver\spi_master.h
//sory linux bros i don't know where that is on here


void setup() {


//init hspi gigahell
spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPANBUF_SIZE * 3 + 16
    };
      esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    Serial.printf("spi_bus_initialize failed: %d\n", ret);
    while (1);
  }
    // On ESP32-S3, use SPI2_HOST for what Arduino calls HSPI
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));


// 1) Attach OLED device
spi_device_interface_config_t oledcfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,
    .clock_source = SPI_CLK_SRC_DEFAULT,
    .duty_cycle_pos = 128,  // 50% duty cycle
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz = 26666666,  // 26.6MHz
    .input_delay_ns = 0,
    .spics_io_num = SPI_CS_OLED,
    .flags = SPI_DEVICE_NO_DUMMY,
    .queue_size = 1,
    .pre_cb = nullptr,
    .post_cb = nullptr
};

    //ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &oledcfg, &oledSpiHandle));

// 2) Attach SD card

    spi_device_interface_config_t sdcfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,
    .clock_source = SPI_CLK_SRC_DEFAULT,
    .duty_cycle_pos = 128,  // 50% duty cycle
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz = 26666666,  // 26.6MHz
    .input_delay_ns = 0,
    .spics_io_num = SPI_CS_SD,
    .flags = SPI_DEVICE_NO_DUMMY,
    .queue_size = 1,
    .pre_cb = nullptr,
    .post_cb = nullptr
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &sdcfg, &sdSpiHandle));

  // === 3) Init Arduino SPI wrapper on same HSPI bus ===
  // This does NOT re-init the bus — it just binds the pins.
  //myHSPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, -1);

  // === 4) Init Adafruit PN532 ===
  /*
  if (!nfc.begin()) {
    Serial.println("PN532 init failed!");
  }
  else{
  Serial.println("PN532 init OK");}*/

app_main();
//allocate buffer for screen writing. yes, it's tiny for low ram use


 //   ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &nfccfg, &nfcSpiHandle));
//end setup struct+cfg
    delay(148); 
    Serial.begin(115200);
    

  delay(100); // Let this Bitch™ boot

   // _dbg_ypos = 0; // Reset debug print position
    //spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    screen_on();
    screen_startup();
    tft_Fillscreen(0x0000);
   // set_orientation(0);

    //DBG_PRINTLN("BOOT BEGIN");

Wire.begin(SDA_PIN, SCL_PIN);

        scanI2C();

    SetupHardwareInput();
    //DBG_PRINTLN("Input OK");
    
/*
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
    }*/

if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    //DBG_PRINTLN("HRSENSORFAILURE");
  }
else{
  Serial.println("Place your index finger or wrist on the sensor with steady pressure.");
//DBG_PRINTLN("hr sensor ok");
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

  
}//end else


    windowManagerInstance = WindowManager::getWinManagerInstance();
    if (!windowManagerInstance) {
        //DBG_PRINTLN("WinMgr FAIL");
        return;
    } else {
        //DBG_PRINTLN("WinMgr OK");
    }



        lockscreen_clock = std::make_shared<Window>("lockscreen_clock", d_ls_c_cfg, "HH:MM:SS");
    windowManagerInstance->registerWindow(lockscreen_clock);
    //DBG_PRINTLN("Clock OK");

            lockscreen_biomon = std::make_shared<Window>("lockscreen_biomon", d_ls_b_cfg, "XXXbpm");
    windowManagerInstance->registerWindow(lockscreen_biomon);
    //DBG_PRINTLN("Biomon OK");

        lockscreen_thermometer = std::make_shared<Window>("lockscreen_thermometer", d_ls_th_cfg, "XXXC");
    windowManagerInstance->registerWindow(lockscreen_thermometer);


   // //DBG_PRINTLN("Thermo OK");
   
//SetDeviceTheme(Current_Theme);//change the color palette refs

TFillRect(0,0,128,128,0x0000);//black screen out


//windowManagerInstance->ApplyThemeAllWindows(tcol_secondary, tcol_background, tcol_primary); //with new vars



processInputQueue = xQueueCreate(8, sizeof(S_UserInput)); //set up default que
xTaskCreate(watchscreen, "WatchScreen", 4096, NULL, 1, NULL);//core 0 watch screen 


    ////DBG_PRINTLN("watchscreen task OK");
    xTaskCreatePinnedToCore(INPUT_tick, "INPUT_tick", 2048, NULL, 2, NULL, 1); //core 1 sensor updates


//evil spagetti
//processInputQueue = lockscreenQueue;//2. tell input router to use that que
currentinputTarget = R_toProc; //3. MANUALLY alter input handling values to route to proscesses. we



    //DBG_PRINTLN("SETUP DONE");
    delay(100);

}//end void setup


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
                    WatchScreenUpdateInterval=100;//update WAY more frequently at 100ms
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