#ifndef WIRING_H
#define WIRING_H
//conf for wiring

//device chip, esp32-s3
//http://wiki.fluidnc.com/en/hardware/ESP32-S3_Pin_Reference



#define SPI_SCK   12 //may aslo be called clk
#define SPI_CLK 12 //UGH

#define SPI_MOSI  11//aster out slave in
#define SPI_MISO  13//master in slave out


#define BUZZER_PIN 2


//spi needs seperate cs for each chip 
#define SPI_CS_NFC 10 //PN532
#define SPI_CS_OLED 17//SSD 1351 oled display, 128*128 16 bit color
//#define SPI_CS_RADIO //cc1101 chipset 
#define SPI_CS_SD 6 //generic sdcard on g6


//i2c pins
//hardware known connected: max30012 hr sensor, mpu6050 gyro, 
#define SDA_PIN 8
#define SCL_PIN 9//if i read the data right these are also acessable by the ulp co-proscessor

//other pins
#define IRQ 16 //notify from input devices

#define OLED_DC 4
#define OLED_DC_DIRECT_REF GPIO_NUM_4
#define OLED_RST 5
#define NFC_RST_PIN 15//uhhhhhhhhhhh

#define NEOPIXEL_PIN 48 

//warning undefined
#define IR_IO_Pin 46

//you may be wondering why i'm abstracting this. keep wondering. 

#define million 1000000

/*
void StartSPI() {
  //SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  spiBus.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
}*/

//msc hardware configurations
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 
#define SPI_FREQUENCY 28*million 
#define SPI_FREQUENCY_OLED 18000000 //used for spi burst mode things 

//custom hal. bloat is BAD (shocker) but this is arduino ide so we have to take off the baby shoes because pio is evil makefile hell
#define FGPIO_HIGH(pin) REG_WRITE(GPIO_OUT_W1TS_REG, BIT(pin))
#define FGPIO_LOW(pin)  REG_WRITE(GPIO_OUT_W1TC_REG, BIT(pin))
//hardware testing subsection


//x axis,sel
#define ENCODER0_CLK_PIN 42
#define ENCODER0_DT_PIN  14
#define ENCODER0_SW_PIN  7

//y axis, back

#define ENCODER1_CLK_PIN 21
#define ENCODER1_DT_PIN  47
#define ENCODER1_SW_PIN  18



//periphrials
//#define PWR_NFC 45 //this gipo can't be used on boot, which is why i got ts right here for nfc which will only be powered after boot. also we don't have the hardware for that rn but whateves

#define IR_INPUT_PIN 46 //input-only and must be used only after boot - it is a strapping pin primarily used for boot mode configuration and enabling/disabling ROM messages during booting

//some warnings:

//usb hardware occupies gipo 19 and 20

/*


byte knownAddresses[] = {0x68, 0x24, 0x57}; // Known hardware addresses of the sensors over I2C
int knownDeviceCount = sizeof(knownAddresses) / sizeof(knownAddresses[0]); // Number of known devices

void scanI2C();
const char* getDeviceName(byte address);
void scanSPI();
bool checkSPIDevice(int csPin);

void scanI2C() {
  byte error, address;
  int nDevices = 0;
  bool deviceFound[knownDeviceCount] = {false}; // Keep track of whether known devices are found

  tft.setCursor(0, 0);
  tft.println("I2C scan:");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      tft.setTextColor(GREEN);
      tft.printf("0x%02X  %s  OK\n", address, getDeviceName(address));
      nDevices++;

      // Check if the detected address matches a known address
      for (int i = 0; i < knownDeviceCount; i++) {
        if (address == knownAddresses[i]) {
          deviceFound[i] = true; // Mark this device as found
        }
      }
    } else if (error == 4) {
      tft.setTextColor(RED);
      tft.printf("0x%02X  %s  ERROR\n", address, getDeviceName(address));
    }
  }

  if (nDevices == 0) {
    tft.setTextColor(RED);
    tft.println("No I2C devices found");
  }

  // Now check if any known devices are missing
  for (int i = 0; i < knownDeviceCount; i++) {
    if (!deviceFound[i]) {
      tft.setTextColor(RED);
      tft.printf("MISSING %s\n", getDeviceName(knownAddresses[i]));
    }
  }
}

const char* getDeviceName(byte address) {
  switch (address) {
    case 0x68: return "ACCL/tmp"; // Accelerometer 
    case 0x24: return "NFC/RFID"; // PN532 NFC/RFID module
    case 0x57: return "Heart Rate";
    //case 0x0C: return "Magnetometer";
   // case 0x48: return "Temperature";
    // Add more known I2C device addresses here with their names
    default: {
      static char unknownAddress[10];
      snprintf(unknownAddress, sizeof(unknownAddress), "0x%02X", address);
      return unknownAddress;
    }
  }
}


void scanSPI() {
  // Add SPI device status checking here
  // For example purposes, we'll simulate checking two devices
  tft.setCursor(0, 64);
  tft.setTextColor(WHITE);
  tft.println("SPI scan:");
    tft.setTextColor(MAGENTA);
  tft.println("i'll do this thing later");
 
}

bool checkSPIDevice(int csPin) {
  // Example code to check SPI device status
  // Use csPin to select the appropriate SPI device
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, LOW);
  // Perform some SPI communication to check the device status
  digitalWrite(csPin, HIGH);

  // Return true if the device is working, otherwise false
  // For now, we'll just simulate by returning true
  return true;
}






void listTasks() {
    const int maxTasks = 10; // Adjust based on expected number of tasks
    TaskStatus_t taskStatus[maxTasks];
    UBaseType_t taskCount;
    char taskListBuffer[512]; // Buffer for task list

    // Get number of tasks and their states
    taskCount = uxTaskGetSystemState(taskStatus, maxTasks, NULL);

    Serial.println("Task Name\tState\tPriority\tStack\tTask Number");
    for (int i = 0; i < taskCount; i++) {
        Serial.printf("%s\t%c\t%u\t%u\t%u\n", 
                      taskStatus[i].pcTaskName,
                      taskStatus[i].eCurrentState == eRunning ? 'R' :
                      taskStatus[i].eCurrentState == eReady ? 'Y' :
                      taskStatus[i].eCurrentState == eBlocked ? 'B' :
                      taskStatus[i].eCurrentState == eSuspended ? 'S' : 'D',
                      taskStatus[i].uxCurrentPriority,
                      taskStatus[i].usStackHighWaterMark,
                      taskStatus[i].xTaskNumber);
    }
}

void checkFreeRAM() {
    size_t freeRAM = esp_get_free_heap_size();
    Serial.printf("Free RAM: %u bytes\n", freeRAM);
}

*/



#endif