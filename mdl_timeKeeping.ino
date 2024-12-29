#ifndef mdl_timeKeeping_H
#define mdl_timeKeeping_H

#include <time.h>
#include <sys/time.h>

// Global variables for time access
extern int currentHour;
extern int currentMinute;
extern int currentSecond;

// Global variable for Unix time
extern time_t currentUnixTime;

/*
struct tm is defined like this by default in the code for the esp32 because apparently it's slightly unix compatable or whatever
extern struct tm {
  int tm_sec;   // Seconds
  int tm_min;   // Minutes
  int tm_hour;  // Hours
  int tm_mday;  // Day of the month
  int tm_mon;   // Month (0-11)
  int tm_year;  // Year (since 1900)
  int tm_wday;  // Day of the week (0-6)
  int tm_yday;  // Day of the year (0-365)
  int tm_isdst; // Daylight saving time flag
};
*/

//struct for time itself
struct FreindlyTime { //i renamed all instances of dateTime to FreindlyTime. and i replaced it with the find n replace. aha. gl nerd
    int year;
    int month;  // 1-12
    int day;    // 1-31
    int hour;   // 0-23
    int minute; // 0-59
    int second; // 0-59
};

//todo make sure i have timezones working right, and daylight savings (daylight savings really shouldn't exist, ugh. why was it ever created)

// Initialize the RTC with stored Unix time or set a default Unix time. probably needs to be fixed.
void initializeRTC() {
    if (preferences.isKey("unix_time")) {
        // Retrieve stored Unix time from NVS
        time_t storedTime = preferences.getULong("unix_time");

        // Set the RTC time directly using Unix time
        struct timeval now = { .tv_sec = storedTime, .tv_usec = 0 };
        settimeofday(&now, NULL);

        Serial.println("Time initialized from NVS");
    } else {
        // No time stored, setting default Unix time (e.g., Jan 1, 2024, 00:00 UTC)
        time_t defaultUnixTime = 1704067200;  // Example: Jan 1, 2024, 00:00 UTC

        // Set the RTC time directly using default Unix time
        struct timeval now = { .tv_sec = defaultUnixTime, .tv_usec = 0 };
        settimeofday(&now, NULL);

        Serial.println("No time stored"); //TODO: make sure this sets a default
    }
}

// Update the stored Unix time in NVS
//should get graceful faulure handling later
void updateStoredTime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    currentUnixTime = now.tv_sec;

    // Store the current Unix time in NVS
    preferences.putULong("unix_time", currentUnixTime);
    Serial.println("Time updated in NVS");
}

// Print the current local time
void printLocalTime() {
    struct tm timeinfo;
    time_t now;
    time(&now);

    if (!localtime_r(&now, &timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    // Print the date and time
    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%Y/%b/%d", &timeinfo);
    Serial.print("Date: ");
    Serial.println(dateStr);

    char time24Str[10];
    strftime(time24Str, sizeof(time24Str), "%H:%M:%S", &timeinfo);
    Serial.print("Time (24-hour): ");
    Serial.println(time24Str);

    char time12Str[10];
    strftime(time12Str, sizeof(time12Str), "%I:%M:%S %p", &timeinfo);
    Serial.print("Time (12-hour): ");
    Serial.println(time12Str);
}

// Convert Unix time to local FreindlyTime with time zone support
void unixTimeToFreindlyTime(time_t unixTime, int timeZoneOffset, FreindlyTime &FreindlyTime) {
    // Adjust for the time zone offset (in seconds)
    unixTime += timeZoneOffset;

    // Convert to a tm struct
    struct tm timeinfo;
    gmtime_r(&unixTime, &timeinfo);

    // Populate the FreindlyTime structure
    FreindlyTime.year = timeinfo.tm_year + 1900;
    FreindlyTime.month = timeinfo.tm_mon + 1;
    FreindlyTime.day = timeinfo.tm_mday;
    FreindlyTime.hour = timeinfo.tm_hour;
    FreindlyTime.minute = timeinfo.tm_min;
    FreindlyTime.second = timeinfo.tm_sec;
}

// Convert FreindlyTime to Unix time with time zone adjustment
time_t FreindlyTimeToUnixTime(const FreindlyTime &FreindlyTime, int timeZoneOffset) {
    struct tm timeinfo = {};
    timeinfo.tm_year = FreindlyTime.year - 1900;
    timeinfo.tm_mon = FreindlyTime.month - 1;
    timeinfo.tm_mday = FreindlyTime.day;
    timeinfo.tm_hour = FreindlyTime.hour;
    timeinfo.tm_min = FreindlyTime.minute;
    timeinfo.tm_sec = FreindlyTime.second;

    // Convert to Unix time and adjust for time zone offset
    return mktime(&timeinfo) - timeZoneOffset;
}

// Provide normalized local time (FreindlyTime struct) from Unix time
void GetNormalTime(time_t unixTime, int timeZoneOffset, FreindlyTime &FreindlyTime) {
    unixTimeToFreindlyTime(unixTime, timeZoneOffset, FreindlyTime);
}

FreindlyTime timeUntil(const FreindlyTime &current, const FreindlyTime &target) { //time a to time b, freindly time
    time_t currentUnix = FreindlyTimeToUnixTime(current, timeZoneOffset);
    time_t targetUnix = FreindlyTimeToUnixTime(target, timeZoneOffset);

    time_t diff = targetUnix - currentUnix;
    FreindlyTime result;
    unixTimeToFreindlyTime(diff, 0, result); // Offset = 0 since this is a duration
    return result;
}

enum days{
  mon,
  tue,
  wend,
  thur,
  fri,
  sat,
  sun
};

// Define weekdays and weekends
const bool isWeekday[7] = { true, true, true, true, true, false, false }; // Mon-Fri: true; Sat, Sun: false


enum months{
  jan,
  feb,
  mar,
  jun,
  jul,
  sept,
  oct,
  nov,
  dec
};

//more advanced shitty features for watches defaults.
struct Alarm { //a repeating alarm that happens at a certain point each day
    bool allowSnooze; //should we even let users hit snooze?
    bool forceKeepDeviceOn; //dicks with power settings. fuck you, you're waking up today
    bool isEnabled;
    int SnoozeDurationMin; //min of snooze duration default
    enum days activeDays[7];//array of days we want this on // Specific days for the alarm (up to 7, gaps allowed)
    int loudness; //a 0-100 value for percents of loudness
    bool Flash_Led; //flash the led if 
    bool Flash_screen; //flash the screen or something idk?
    bool pushNotifToConnectedDevice; //ping your phone too, i guess. 

};
//half these options do nothing just yet because unfinished hardware and software but that's fine i guess. 


bool isAlarmTime(struct Alarm *alarm, enum days currentDay, int currentHour, int currentMinute) {
    if (!alarm->isEnabled) return false;

    // Check if today is in the activeDays array
    for (int i = 0; i < alarm->numActiveDays; i++) {
        if (alarm->activeDays[i] == currentDay) {
            // Check time match
            if (alarm->hour == currentHour && alarm->minute == currentMinute) {
                return true;
            }
        }
    }
    return false;
}

void checkAlarms(struct Alarm *alarms, int numAlarms, enum days currentDay, int currentHour, int currentMinute) {
    for (int i = 0; i < numAlarms; i++) {
        if (isAlarmTime(&alarms[i], currentDay, currentHour, currentMinute)) {
            triggerAlarm(&alarms[i]);
        }
    }
}

//ASSIGN THE alarm task to freerots task scheduling. 
//todo: set low priority task so it doesn't work too hard. this also needs to run during sleep modes unless we do periodic boots back to half awake? 
int AlarmAssignBytes=512;

void checkAlarmsWithTask(struct Alarm *alarms, int numAlarms, enum days currentDay, int currentHour, int currentMinute) {
    for (int i = 0; i < numAlarms; i++) {
        if (isAlarmTime(&alarms[i], currentDay, currentHour, currentMinute)) {
            // Create a new FreeRTOS task for the alarm
            xTaskCreate(alarmTask, "AlarmTask", alarmAssignBytes, &alarms[i], tskIDLE_PRIORITY, NULL); 
        }
    }
}


//TODO: MOVE THESE ALARM FUNCTIONS TO THE STORAGE MODULE


//store your allarms at nvs. 
void saveAlarmsToNVS(struct Alarm *alarms, int numAlarms) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("alarm_storage", NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        // Write alarms to NVS
        for (int i = 0; i < numAlarms; i++) {
            char key[16];
            snprintf(key, sizeof(key), "alarm_%d", i); // Create unique key
            nvs_set_blob(nvsHandle, key, &alarms[i], sizeof(struct Alarm));
        }
        nvs_commit(nvsHandle);
        nvs_close(nvsHandle);
    }
}

//call this on boot to load alarms
int loadAlarmsFromNVS(struct Alarm *alarms, int maxAlarms) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("alarm_storage", NVS_READONLY, &nvsHandle);
    int alarmCount = 0;

    if (err == ESP_OK) {
        for (int i = 0; i < maxAlarms; i++) {
            char key[16];
            snprintf(key, sizeof(key), "alarm_%d", i);

            size_t alarmSize = sizeof(struct Alarm);
            if (nvs_get_blob(nvsHandle, key, &alarms[i], &alarmSize) == ESP_OK) {
                alarmCount++;
            } else {
                break; // Stop on first missing alarm
            }
        }
        nvs_close(nvsHandle);
    }
    return alarmCount;
}






//guess what, we don't have the logic for this but that's fine
void triggerAlarm(struct Alarm *alarm) {
    if (alarm->flashLed) {
        // Flash the LED
    }
    if (alarm->flashScreen) {
        // Flash the screen
    }
    if (alarm->pushNotifToConnectedDevice) {
        // Push notification
    }
    // Play alarm sound at specified loudness
}










//timers 

struct Stopwatch {
    time_t startUnixTime;
    time_t stopUnixTime;
    bool isRunning;
};

//Stopwatch stopwatch1;  //tip, set this to set up a stopwatch

void startStopwatch() {
    stopwatch.startUnixTime = getUnixTime();
    stopwatch.isRunning = true;
}

void stopStopwatch() {
    if (stopwatch.isRunning) {
        stopwatch.stopUnixTime = getUnixTime();
        stopwatch.isRunning = false;
    }
}

time_t getElapsedTime() { 
    return stopwatch.isRunning ? (getUnixTime() - stopwatch.startUnixTime)
   (stopwatch.stopUnixTime - stopwatch.startUnixTime);
}

//todo: store the stopwatch? hell we don't need to do that, if it boots down it wasn't that important was it now.


//end the module
}

#endif // MDL_TIMEKEEPING_H
