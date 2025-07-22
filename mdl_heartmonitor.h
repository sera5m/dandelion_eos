#ifndef mdl_heartmonitor_H
#define mdl_heartmonitor_H

#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

// External sensor object
extern MAX30105 particleSensor;

// Heart rate data structure
struct HeartRateData {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int16_t heartRate;
};

// Time variables
extern int currentHour;
extern int currentMinute;
extern int currentSecond;

// User temperature variable
extern float usertemperature;

// Heart rate thresholds (configurable)
extern int HR_WALKING_THRESHOLD;
extern int HR_SLEEP_THRESHOLD;
extern int HR_RESTING_THRESHOLD;
extern int HR_EXERCISE_THRESHOLD;
extern int HR_EXCESSIVE_THRESHOLD;

// Heart rate processing
#define RATE_SIZE 9
extern byte rates[RATE_SIZE];
extern byte rateSpot;
extern unsigned long lastBeat;
extern float beatsPerMinute;
extern int beatAvg;
extern int AVG_HR;
extern int16_t HRminAverage;
extern bool isFirstBeat;
extern long lastValidBPM;
extern const unsigned long MIN_BEAT_INTERVAL;

// Blood oxygen processing
extern bool enableBloodOxygen;
extern const int BO_THRESHOLD;

// Sensor state enum
enum SensorState {
    NO_FINGER,
    DETECTING,
    STABLE_READING
};

extern SensorState currentState;
extern unsigned long lastStateChange;
extern const unsigned long HR_MONITOR_DEBOUNCE_TIME;

// Function declarations
int hr_guess_usr_activity(int bpm);
void checkBodyTemp();
bool HRsensorSetup();
void Log_heartRateData();
bool BeatCheck(long irValue);
void updateHRsensor();
long denoiseIR(long irValue);

#endif // mdl_heartmonitor_H