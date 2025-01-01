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


//stuff
int timeZoneOffset = -1; // Example default value for timezone (i have utc-1 by default  )



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

//struct for time in a nice human readable format
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
//tip: use above to convert to time and date from unix ticks, or convert unix tick to the normal time. i think. 

//use this function to convert seconds (say from a timer) to normative time structures. uses mod to provide ms as a remainder
//i don't know this works, i glued it together. VERY sloppily
void RawSecondsToFreindlyTime(float seconds, FreindlyTime &friendlyTime, float &ms) {
    // Break the float into whole seconds and the fractional part
    float fractionalPart = 0.0f;
    float wholeSeconds = std::modf(seconds, &fractionalPart); // Fractional part gives us the remainder
    
    // Convert fractional seconds to milliseconds (1 second = 1000 ms)
    ms = fractionalPart * 1000;
    
    // Now, we need to convert wholeSeconds to friendlytime 
    // using a 1-second precision? i give up
    int totalSeconds = static_cast<int>(wholeSeconds); // Convert the seconds to an integer

    // Handle year, month, day, hour, minute, and second conversion
    // Let's assume we are starting from a reference point (e.g., the Unix epoch: January 1, 1970).
    int secondsInYear = 365 * 24 * 60 * 60; // Simplification, not accounting for leap years.
    int secondsInMonth = 30 * 24 * 60 * 60;  // Simplified month length assumption (30 days).

    // Calculate the year
    friendlyTime.year = 1970 + totalSeconds / secondsInYear;
    totalSeconds %= secondsInYear;  // Remaining seconds after accounting for years

    // Calculate the month
    friendlyTime.month = 1 + totalSeconds / secondsInMonth;
    totalSeconds %= secondsInMonth;  // Remaining seconds after accounting for months

    // Calculate the day
    friendlyTime.day = 1 + totalSeconds / (24 * 60 * 60);  // Days from the start of the month
    totalSeconds %= (24 * 60 * 60);  // Remaining seconds after accounting for days

    // Calculate the hour
    friendlyTime.hour = totalSeconds / 3600;
    totalSeconds %= 3600;  // Remaining seconds after accounting for hours

    // Calculate the minute
    friendlyTime.minute = totalSeconds / 60;
    totalSeconds %= 60;  // Remaining seconds after accounting for minutes

    // The remaining seconds are the exact second value
    friendlyTime.second = totalSeconds;
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
void GetFreindlyTime(time_t unixTime, int timeZoneOffset, FreindlyTime &FreindlyTime) {
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




//*****************************************************************************************************
//clock features
//*****************************************************************************************************


//todo: this needs mercury interaction, and visuals.
//visuals will be made after The visual module revamp, because this module will require popups, an actual window with canvas, and the better formatting for the stopwatch













//more advanced shitty features for watches defaults.
struct Alarm { //a repeating alarm that happens at a certain point each day
       bool isEnabled;
   freindlytime //the freindly time structure TODO FIX THIS! THIS NEEDS A FREINDLY TIME STRUCTURE WITH NO DAY VAR I THINK. idk. 
    enum days activeDays[7];//array of days we want this on // Specific days for the alarm (up to 7, gaps allowed)
    int loudness; //a 0-100 value for percents of loudness
    bool Flash_Led; //flash the led if 
    bool Flash_screen; //flash the screen or something idk?
    bool pushNotifToConnectedDevice; //ping your phone too, i guess. 
     bool allowSnooze; //should we even let users hit snooze?
    bool forceKeepDeviceOn; //dicks with power settings. fuck you, you're waking up today
     int SnoozeDurationMin; //min of snooze duration default

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
#define AlarmAssignBytes=512;



void alarmTask(void* parameter) {
    Alarm* alarm = (Alarm*)parameter;
    // Alarm handling logic
    vTaskDelete(NULL);
}



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





/*
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
*/

//todo: add a TIMER: a non repeating alarm that happens once and then deletes itself




//std chrono is a regular c library yo handle time
//it's got the high prescision stuff we need. because users of a watch need high prescision time.

class Stopwatch {
private:
    using high_res_clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<high_res_clock>;

    time_point startTime;
    time_point stopTime;
    bool isRunning;
    std::vector<double> laps; // Lap times in seconds with high precision (up to 6 laps)
    //if this causes optimization issues, we will need to replace this with a fixed size array. probably a todo, i doubt we'll need more than 3.

    double currentElapsed() const {
        if (isRunning) {
            auto now = high_res_clock::now();
            return std::chrono::duration<double>(now - startTime).count();
        } else {
            return std::chrono::duration<double>(stopTime - startTime).count();
        }
    }

public:
    // Constructor
    Stopwatch() : isRunning(false) {}

    // Start the stopwatch
    void start() {
        if (!isRunning) {
            startTime = high_res_clock::now(); 
            isRunning = true;
        }
    }

    // Stop the stopwatch
    void stop() {
        if (isRunning) {
            stopTime = high_res_clock::now();
            isRunning = false;
        }
    }

//reset this
void reset() {
    isRunning = false;
    startTime = stopTime = time_point{}; // Ensure no leftover times
    laps.clear();
}



    // Record a lap
    bool recordLap() { 
        if (laps.size() >= 6) return false; // Maximum 6 laps. hardcoded for now, because why not? 
        double elapsed = currentElapsed();
        laps.push_back(elapsed); 
        return true;
    }

  //get elapsed time
    double getElapsedTime() const {
    double totalLapsTime = 0.0;
    for (double lap : laps) {
        totalLapsTime += lap;
    }
    return currentElapsed() + totalLapsTime;
}


    // Get lap time by index (returns nullptr if invalid index)
    std::optional<double> getLap(size_t index) const { //this should see if it exists before returning the value. 
    if (index < laps.size()) { 
        return laps[index]; 
    }
    return std::nullopt; //null if no index is available
}


    // Print all laps. debug for now, add an option to print to screen or not later.

    void printLaps() const {
    if (laps.empty()) {
        std::cout << "No laps recorded.\n";
        return;
    }
    for (size_t i = 0; i < laps.size(); ++i) {
        Serial.println << "Lap " << i + 1 << ": " << std::fixed << std::setprecision(4) << laps[i] << " seconds\n";
    }
}





};

//to use the stopwatch:

//create object: Stopwatch [name];
//start timer: [object name].start();
//record laps: [object name].recordLap();
//stop timer: [object name].stop();
//print laps: [object name].printLaps();



//

//end the module
}

#endif // MDL_TIMEKEEPING_H
