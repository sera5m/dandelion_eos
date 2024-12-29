#ifndef mdl_heartmonitor_H
#define mdl_heartmonitor_H

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

/*
define the following functions:

-check user presence
-heart rate (ignore junk data)
-is sensor secured on arm?
-denoising
-averaging heart rate
-store 1 data point per minute for averaging on hard storage (eg: [timestamp] avghr: [value]) as simple data. note that the datapoints for every few seconds are kept in ram though and can be streamed to a phone continuously if enabled, uses bt module (a send packet function not in this module)
-activity detection (excersize,rest, sleep, etc)
-use of blood oxygenation data, (elaborate on purposes)
-user calibration for watch, for the purpose of knowing mood or whatever i don't know really


*/
// Declare the global variables here
float usertemperature; // Global variable, no extern needed unless used in another file
// Global variable for user temperature. this is the USERS temperature as of what the infared sensor says. DO NOT CONFUSE WITH DEVICETEMP!



//private vars to this class

//thresHolds of heart rate. (all ints) for various activities. should be exposed to the user to let em config
//TODO: expose these for the user and load em from hard storage if they're there!
int HR_sleepThresHold=45;
int HR_restingThresHold=65;
int HR_walkingThresHold=100;
int HR_excersiseThresHold=140;
int HR_excessiveHR=180;


// Sensor state
bool isWorn;
const byte RATE_SIZE = 9; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float beatsPerMinute;
int beatAvg; //should be the user's average heart rate

// Minimum average heart rate storage
int16_t HRminAverage = 1; // This sensor will average your heart rate each minute, then add it to storage once per minute for health tracking (1440 samples per day)

//i'll have to tick this every little while to guess what the user is doing. 

int hr_guess_usr_activity(int bpm) {
    if (beatAvg < HR_walkingThresHold) { //cut this in half through the middle to half if comparisons
        if (beatAvg < HR_sleepThresHold) {
            return 0; // Below sleep threshold
        } else if (beatAvg < HR_restingThresHold) {
            return 1; // Between sleep and resting threshold
        } else {
            return 2; // Between resting and walking threshold
        }
    } else {
        if (beatAvg < HR_excersiseThresHold) {
            return 3; // Between walking and exercise threshold
        } else if (beatAvg < HR_excessiveHR) {
            return 4; // Between exercise and excessive heart rate
        } else {
            return 5; // Above excessive heart rate
        }
    }
}


void checkbodytemp()
{
    // Now just assign a new value to the global variable
    usertemperature = particleSensor.readTemperature();

    // Print the temperature in Celsius
    Serial.print("temperatureC=");
    Serial.print(usertemperature, 2);  // Using 2 decimals for low precision

    // If you want to use Fahrenheit, you can calculate it like this:
    // float usertemperatureF = particleSensor.readTemperatureF(); 

    Serial.println();
}



void HRsensorSetup()
{

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger or wrist on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}






void Log_heartrateData(int16_t HRminAverage) //call this once per min, i guess. may be out of date for modifications to mdl time keeping mods and log changes
{
    HeartRateData data;

    // Fill with current timestamp values from the external variables
    data.year = 2024;       // Example: fixed year (replace with actual time function if available)
    data.month = 10;        // Example month (October, replace with actual value if needed)
    data.day = 5;           // Example day (replace with actual date if needed)
    data.hour = currentHour;   // External variable for the current hour
    data.minute = currentMinute; // External variable for the current minute
    data.second = currentSecond; // External variable for the current second
    data.heartRate = beatAvg;  // Use beatAvg as the heart rate value
}

int AVG_HR=1; //the average heart rate
#define FILTER_SIZE 5  // Define the size of the window for moving average
#define MIN_BEAT_INTERVAL 400  // Minimum time (in ms) between beats to avoid false positives


long irValueBuffer[FILTER_SIZE] = {0};  // Buffer to hold IR values for denoising
int bufferIndex = 0;
long lastValidBPM = 0;  // Store the last BPM that was considered valid
bool isFirstBeat = true;  // Track the first beat for initialization

// Function to denoise IR values using a moving average
long denoiseIR(long irValue) {
    irValueBuffer[bufferIndex++] = irValue;
    bufferIndex %= FILTER_SIZE;

    long sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += irValueBuffer[i];
    }
    return sum / FILTER_SIZE;
}

void updateheartrate() {
    long rawIRValue = particleSensor.getIR();
    long irValue = denoiseIR(rawIRValue);  // Apply denoising

    long currentTime = millis();
    
    // Beat detection logic
    if (checkForBeat(irValue) == true) {
        long delta = currentTime - lastBeat;
        
        // Reject beats that are too close together to avoid false positives
        if (delta > MIN_BEAT_INTERVAL) {
            lastBeat = currentTime;
            beatsPerMinute = 60 / (delta / 1000.0);

            // Filter unreasonable BPM values
            if (beatsPerMinute < 255 && beatsPerMinute > 20) {
                rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading
                rateSpot %= RATE_SIZE;

                // Take average of the last readings
                beatAvg = 0;
                for (byte x = 0; x < RATE_SIZE; x++) {
                    beatAvg += rates[x];
                }
                beatAvg /= RATE_SIZE;

                // Smooth out large jumps in BPM by limiting the rate of change
                if (!isFirstBeat) {
                    if (abs(beatAvg - lastValidBPM) > 10) {
                        beatAvg = lastValidBPM + ((beatAvg - lastValidBPM) * 0.5);  // Smoothing jump
                    }
                } else {
                    isFirstBeat = false;
                }

                lastValidBPM = beatAvg;  // Store this BPM as the last valid one
            }
        }
    }
/*
    // Output results
    Serial.print("IR=");
    Serial.print(irValue);  // Output the denoised IR value
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
    */
     AVG_HR=beatAvg; //SET THE var for ext use

    if (irValue < 50000){
        Serial.print(" No finger?");
        AVG_HR = 0;

    Serial.println();
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
