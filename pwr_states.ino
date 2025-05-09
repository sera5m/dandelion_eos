#ifndef PWR_STATES_H
#define PWR_STATES_H

//weird vars for handling power
bool ForceKeepDeviceOn=false; //forcibly keep the device on, ignoring the users settings. only expose this over a terminal or something
//TODO: expose this for use later

// Global variables for Wi-Fi and Bluetooth state control
extern bool wifiOff;
extern bool btOff;

// Set core frequency function
void setCoreFrequency(float frequencyMHz) {
  // Clamp the frequency within the acceptable range (80 - 240 MHz)
  frequencyMHz = fmax(80.0f, fmin(240.0f, frequencyMHz));

  // Configure the dynamic frequency scaling (DFS) settings
  esp_pm_config_t pm_config;
  pm_config.max_freq_mhz = frequencyMHz;
  pm_config.min_freq_mhz = frequencyMHz;
  pm_config.light_sleep_enable = false; // Disable light sleep to maintain the frequency

  // Apply the new frequency settings
  esp_err_t result = esp_pm_configure(&pm_config);

  if (result != ESP_OK) {
    Serial.printf("Failed to set CPU frequency, error: %d\n", result);
  }
}



// Enter light sleep function
void enterLightSleep(bool wifiOff, bool btOff) {
  if (wifiOff) {
    esp_wifi_stop();
  }

  if (btOff) {
    esp_bt_controller_disable();
  }

  Serial.println("Entering light sleep");
  esp_light_sleep_start();
  Serial.println("Woke up from light sleep");
  
  if (wifiOff) {
    esp_wifi_start();
  }

  if (btOff) {
    esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  }
}

// Enter deep sleep function
void enterDeepSleep() {
  Serial.println("Entering deep sleep");
  esp_deep_sleep_start();
}

// Wake up functions
void wakeUpPart() {
  Serial.println("Waking up from sleep");
  // DOES NOT Reinitialize Wi-Fi and Bluetooth if they were turned off. PRESERVES STATES!
  Serial.println("Partial wakeup complete");
}

void wakeUpAll() {
  Serial.println("Waking up from sleep");
  // Reinitialize Wi-Fi and Bluetooth if they were turned off
  if (wifiOff) {
    esp_wifi_start();
  }

  if (btOff) {
    esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  }

  Serial.println("Full wakeup complete");
}

// Print wakeup reason from deep sleep
void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     
      Serial.println("Wakeup caused by external signal using RTC_IO"); 
      break;
    case ESP_SLEEP_WAKEUP_EXT1:     
      Serial.println("Wakeup caused by external signal using RTC_CNTL"); 
      break;
    case ESP_SLEEP_WAKEUP_TIMER:    
      Serial.println("Wakeup caused by timer"); 
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: 
      Serial.println("Wakeup caused by touchpad"); 
      break;
    case ESP_SLEEP_WAKEUP_ULP:      
      Serial.println("Wakeup caused by ULP program"); 
      break;
    default:                        
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); 
      break;
  }
}

#endif // MDL_PWR_MNGR_H
