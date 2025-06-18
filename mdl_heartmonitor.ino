#ifndef mdl_heartmonitor_H
#define mdl_heartmonitor_H
#include "MAX30105.h" //sparkfun lib
#include "heartRate.h"
#include "spo2_algorithm.h"//aw lawd
extern MAX30105 particleSensor;
struct HeartRateData
{
    uint16_t year;  // 2 bytes
    uint8_t month;  // 1 byte
    uint8_t day;    // 1 byte
    uint8_t hour;   // 1 byte
    uint8_t minute; // 1 byte
    uint8_t second; // 1 byte
    int16_t heartRate; //2 bytes to store values even if they're super high
};

extern int currentHour;
extern int currentMinute;
extern int currentSecond;


// Declare the global variables here
float usertemperature; // Global variable, no extern needed unless used in another file
// Global variable for user temperature. this is the USERS temperature as of what the infared sensor says. DO NOT CONFUSE WITH DEVICETEMP!



//private vars to this class

//thresHolds of heart rate. (all ints) for various activities. should be exposed to the user to let em config
//TODO: expose these for the user and load em from hard storage if they're there!
int HR_WALKING_THRESHOLD = 100;
int HR_SLEEP_THRESHOLD = 45;
int HR_RESTING_THRESHOLD = 65;
int HR_EXERCISE_THRESHOLD = 140;
int HR_EXCESSIVE_THRESHOLD = 180;

//---------------------------------------------------
// Heart Rate Processing State
//---------------------------------------------------
const byte RATE_SIZE = 9;
byte rates[RATE_SIZE] = {0};  // circular buffer for recent BPM readings
byte rateSpot = 0;          // current index in the circular buffer
unsigned long lastBeat = 0; // timestamp of the last beat
float beatsPerMinute = 0;   // instantaneous BPM
int beatAvg = 0;            // moving average BPM (heart rate)
int AVG_HR = 0;             // global average heart rate, updated per beat
int16_t HRminAverage = 1;   // for storing a one-minute average heart rate
bool isFirstBeat = true;
long lastValidBPM = 0;      // used for smoothing rapid changes

// Minimum time (in ms) between beats to avoid false detection
const unsigned long MIN_BEAT_INTERVAL = 200; //even at 200 bpm the users heart rate is 300ms interval

//---------------------------------------------------
// IR Signal Denoising (Moving Average Filter)
//---------------------------------------------------
const int FILTER_SIZE = 5;
long irValueBuffer[FILTER_SIZE] = {0};
int filterIndex = 0;

long denoiseIR(long irValue) {
  irValueBuffer[filterIndex] = irValue;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  long sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    sum += irValueBuffer[i];
  }
  return sum / FILTER_SIZE;
}

//---------------------------------------------------
// Blood Oxygen Processing Variables
//---------------------------------------------------

typedef uint32_t BOType; //def bloodoxygen datatime as uint32, but we can use 16 bit if absolutely needed


// Variables for averaging blood oxygen values
unsigned long lastBOAvgTime = 0;  // time of last 10-sec average
int boSum = 0;
int boCount = 0;
int minuteBOBuffer[10] = {0};     // holds 10 one-minute averages
int boMinuteIndex = 0;
unsigned long lastTenMinuteTime = 0;
const int BO_THRESHOLD = 95;      // example threshold for low SpO2 warning
bool enableBloodOxygen=false;
//---------------------------------------------------
// Function Declarations
//---------------------------------------------------
int hr_guess_usr_activity(int bpm);
void checkBodyTemp();
bool HRsensorSetup();
void Log_heartRateData();
bool BeatCheck(long irValue);
void updateSensors();

//---------------------------------------------------
// Function Definitions
//---------------------------------------------------

// Given the average BPM (beatAvg), guess user activity level (0 to 5).
// 0: below sleep threshold, 1: sleep to resting, 2: resting, 3: walking/exercise, 4: exercise, 5: excessive.
int hr_guess_usr_activity(int bpm) {
  if (bpm < HR_WALKING_THRESHOLD) {
    if (bpm < HR_SLEEP_THRESHOLD) {
      return 0; // Below sleep threshold
    } else if (bpm < HR_RESTING_THRESHOLD) {
      return 1; // Between sleep and resting threshold
    } else {
      return 2; // Between resting and walking threshold
    }
  } else {
    if (bpm < HR_EXERCISE_THRESHOLD) {
      return 3; // Between walking and exercise threshold
    } else if (bpm < HR_EXCESSIVE_THRESHOLD) {
      return 4; // Between exercise and excessive heart rate
    } else {
      return 5; // Above excessive heart rate
    }
  }
}

// Reads the temperature from the sensor and prints it.
void checkBodyTemp() {
  // Read sensor temperature (Celsius)
  float usertemperature = particleSensor.readTemperature();
  Serial.print("Temperature (C)= ");
  Serial.println(usertemperature, 2);
}

// Sensor initialization: sets up the MAX30105 sensor with appropriate settings.
bool HRsensorSetup() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    return false;
  }

  Serial.println("Place your index finger or wrist on the sensor with steady pressure.");

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

  return true;
}


// Log heart rate data into a HeartRateData struct. (Extend this function to store or send the data.)
void Log_heartRateData() {
  HeartRateData data;
  data.year    = 2024;         // Replace with real-time values as available
  data.month   = 10;
  data.day     = 5;
  data.hour    = currentHour;
  data.minute  = currentMinute;
  data.second  = currentSecond;
  data.heartRate = beatAvg;     // Using the averaged heart rate
                                                                                      //TODO CONVERT TO THE FREINDLY TIME STRUCT OR JUST GET THE ACTUAL TIME
  // For example, print the logged data (or store/send it as needed)
  Serial.print("Logged HR Data: ");
  Serial.print(data.year); Serial.print("-");
  Serial.print(data.month); Serial.print("-");
  Serial.print(data.day); Serial.print(" ");
  Serial.print(data.hour); Serial.print(":");
  Serial.print(data.minute); Serial.print(":");
  Serial.print(data.second);
  Serial.print(" HR=");
  Serial.println(data.heartRate);
}

// Dummy beat detection: returns true if the denoised IR value is above a set threshold.
bool BeatCheck(long irValue) {
  return (irValue > 50000);
}

// updateSensors(): sample the sensor, update heart rate and, if enabled, blood oxygen.
void updateHRsensor() {
  // ----------------- Heart Rate Processing -----------------
  long rawIRValue = particleSensor.getIR();
  long irValue = denoiseIR(rawIRValue);
  unsigned long currentTime = millis();
  
  if (BeatCheck(irValue)) {
    unsigned long delta = currentTime - lastBeat;
    if (delta > MIN_BEAT_INTERVAL) {
      lastBeat = currentTime;
      beatsPerMinute = 60.0 / (delta / 1000.0);
      if (beatsPerMinute > 20 && beatsPerMinute < 255) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        
        // Calculate moving average
        int sum = 0;
        for (byte i = 0; i < RATE_SIZE; i++) {
          sum += rates[i];
        }
        beatAvg = sum / RATE_SIZE;
        
        // Apply smoothing (if not first beat)
        if (!isFirstBeat && abs(beatAvg - lastValidBPM) > 10) {
          beatAvg = lastValidBPM + ((beatAvg - lastValidBPM) * 0.5);
        } else {
          isFirstBeat = false;
        }
        lastValidBPM = beatAvg;
      }
    }
  }
  
  // Update global average heart rate
  AVG_HR = beatAvg;
  
  // Print status if no finger is detected
  if (irValue < 50000) {
    Serial.println("No finger?");
    AVG_HR = 0;
  }
  
  // ----------------- Blood Oxygen Processing -----------------
  if (enableBloodOxygen) {
    // Get the red LED value needed for SpO₂ calculation
    uint32_t rawRedValue = particleSensor.getRed();
    
    // Use static buffers local to this function for 100-sample batches.
    static BOType irBOBuffer[100];
    static BOType redBOBuffer[100];
    static int boBufferIndex = 0;
    
    // Save the current sample
    irBOBuffer[boBufferIndex] = (BOType)irValue;
    redBOBuffer[boBufferIndex] = (BOType)rawRedValue;
    boBufferIndex++;
    
    // When we have 100 samples, process them.
    if (boBufferIndex >= 100) {
      int32_t spo2;
      int8_t validSPO2;
      int32_t dummyHR;
      int8_t validHR;
      maxim_heart_rate_and_oxygen_saturation(irBOBuffer, 100, redBOBuffer,
                                               &spo2, &validSPO2, &dummyHR, &validHR);
      boBufferIndex = 0;  // reset index for next batch
      
      // Accumulate SpO₂ values for averaging over 10 seconds.
      boSum += spo2;
      boCount++;
      unsigned long now = millis();
      if (now - lastBOAvgTime >= 10000) { // Every 10 seconds
        int tenSecAvg = boSum / boCount;
        static int minuteSum = 0;
        static int minuteCount = 0;
        minuteSum += tenSecAvg;
        minuteCount++;
        
        // Reset 10-sec accumulators
        boSum = 0;
        boCount = 0;
        lastBOAvgTime = now;
        
        if (minuteCount >= 6) { // Roughly one minute of data
          int minuteAvg = minuteSum / minuteCount;
          minuteBOBuffer[boMinuteIndex] = minuteAvg;
          boMinuteIndex = (boMinuteIndex + 1) % 10;
          minuteSum = 0;
          minuteCount = 0;
          
          // Every 10 minutes, average the minute averages.
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

#endif // mdl_heartmonitor_H

//this code is somewhat derived from the default library,and hence the chip itself/original lib is from these guys



/* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
* 
*/
