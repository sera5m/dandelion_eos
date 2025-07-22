#include "mdl_heartmonitor.h"
#include <Arduino.h>

// Initialize all the global variables
int HR_WALKING_THRESHOLD = 100;
int HR_SLEEP_THRESHOLD = 45;
int HR_RESTING_THRESHOLD = 65;
int HR_EXERCISE_THRESHOLD = 140;
int HR_EXCESSIVE_THRESHOLD = 180;

// Heart Rate Processing State
byte rates[RATE_SIZE] = {0};
byte rateSpot = 0;
unsigned long lastBeat = 0;
float beatsPerMinute = 0;
int beatAvg = 0;
int AVG_HR = 0;
int16_t HRminAverage = 1;
bool isFirstBeat = true;
long lastValidBPM = 0;
const unsigned long MIN_BEAT_INTERVAL = 200;

// IR Signal Denoising
const int FILTER_SIZE = 5;
long irValueBuffer[FILTER_SIZE] = {0};
int filterIndex = 0;

// Blood Oxygen Processing
unsigned long lastBOAvgTime = 0;
int boSum = 0;
int boCount = 0;
int minuteBOBuffer[10] = {0};
int boMinuteIndex = 0;
unsigned long lastTenMinuteTime = 0;
bool enableBloodOxygen = false;
const int BO_THRESHOLD = 95;

// Sensor State
SensorState currentState = NO_FINGER;
unsigned long lastStateChange = 0;
const unsigned long HR_MONITOR_DEBOUNCE_TIME = 2000;

// Function implementations
long denoiseIR(long irValue) {
  irValueBuffer[filterIndex] = irValue;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  long sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    sum += irValueBuffer[i];
  }
  return sum / FILTER_SIZE;
}

// [Keep all other function implementations the same until updateHRsensor()]

void updateHRsensor() {
  long irValue = denoiseIR(particleSensor.getIR());
  unsigned long currentTime = millis();

  switch(currentState) {
    case NO_FINGER:
      if (irValue > 50000) {
        currentState = DETECTING;
        lastStateChange = currentTime;
      }
      break;
      
    case DETECTING:
      if (irValue < 50000) {
        currentState = NO_FINGER;
      } 
      else if (currentTime - lastStateChange > HR_MONITOR_DEBOUNCE_TIME) {
        currentState = STABLE_READING;
        Serial.println("Finger detected - starting measurements");
      }
      break;
      
    case STABLE_READING:
      if (irValue < 50000) {
        currentState = NO_FINGER;
        AVG_HR = 0;
        Serial.println("Finger removed");
        break;
      }
      
      if (BeatCheck(irValue)) {
        unsigned long delta = currentTime - lastBeat;
        if (delta > MIN_BEAT_INTERVAL) {
          lastBeat = currentTime;
          beatsPerMinute = 60.0 / (delta / 1000.0);
          
          if (beatsPerMinute > 20 && beatsPerMinute < 255) {
            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= RATE_SIZE;
            
            int sum = 0;
            for (byte i = 0; i < RATE_SIZE; i++) {
              sum += rates[i];
            }
            AVG_HR = sum / RATE_SIZE;
          }
        }
      }
      break;
  }

  if (enableBloodOxygen) {
    uint32_t rawRedValue = particleSensor.getRed();
    static uint32_t irBOBuffer[100];  // Changed from BOType to uint32_t
    static uint32_t redBOBuffer[100]; // Changed from BOType to uint32_t
    static int boBufferIndex = 0;
    
    irBOBuffer[boBufferIndex] = (uint32_t)irValue;
    redBOBuffer[boBufferIndex] = (uint32_t)rawRedValue;
    boBufferIndex++;
    
    if (boBufferIndex >= 100) {
      int32_t spo2;
      int8_t validSPO2;
      int32_t dummyHR;
      int8_t validHR;
      maxim_heart_rate_and_oxygen_saturation(irBOBuffer, 100, redBOBuffer,
                                           &spo2, &validSPO2, &dummyHR, &validHR);
      boBufferIndex = 0;
      
      boSum += spo2;
      boCount++;
      unsigned long now = millis();
      if (now - lastBOAvgTime >= 10000) {
        int tenSecAvg = boSum / boCount;
        static int minuteSum = 0;
        static int minuteCount = 0;
        minuteSum += tenSecAvg;
        minuteCount++;
        
        boSum = 0;
        boCount = 0;
        lastBOAvgTime = now;
        
        if (minuteCount >= 6) {
          int minuteAvg = minuteSum / minuteCount;
          minuteBOBuffer[boMinuteIndex] = minuteAvg;
          boMinuteIndex = (boMinuteIndex + 1) % 10;
          minuteSum = 0;
          minuteCount = 0;
          
          if (now - lastTenMinuteTime >= 600000) {
            int sum10 = 0;
            for (int i = 0; i < 10; i++) {
              sum10 += minuteBOBuffer[i];
            }
            int tenMinAvg = sum10 / 10;
            if (tenMinAvg < BO_THRESHOLD) {
              Serial.println("Warning: Blood Oxygen level low!");
            }
            lastTenMinuteTime = now;
          }
        }
      }
    }
  }
}