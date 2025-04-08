#ifndef mdl_diagnosticTools_H
#define mdl_diagnosticTools_H


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

























#endif
