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

extern struct tm { //i think this is the correct unix time thingy,ext so other mdls can use
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

//struct for time itself
struct FreindlyTime { //i renamed all instances of dateTime to FreindlyTime. and i replaced it with the find n replace. aha. gl nerd
    int year;
    int month;  // 1-12
    int day;    // 1-31
    int hour;   // 0-23
    int minute; // 0-59
    int second; // 0-59
};

// Initialize the RTC with stored Unix time or set a default time
void initializeRTC() {
    if (preferences.isKey("unix_time")) {
        // Retrieve stored Unix time
        time_t storedTime = preferences.getULong("unix_time");
        struct timeval now = { .tv_sec = storedTime, .tv_usec = 0 };
        settimeofday(&now, NULL);

        Serial.println("Time initialized from NVS");
    } else {
        Serial.println("No time stored, setting default time");

        // Set default time (e.g., Aug 1, 2024, 12:00 PM UTC)
        struct tm defaultTime = {
            .tm_year = 2024 - 1900,
            .tm_mon = 7, // August (0-based index)
            .tm_mday = 1,
            .tm_hour = 12,
            .tm_min = 0,
            .tm_sec = 0
        };
        time_t defaultUnixTime = mktime(&defaultTime);
        struct timeval now = { .tv_sec = defaultUnixTime, .tv_usec = 0 };
        settimeofday(&now, NULL);
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


#endif // MDL_TIMEKEEPING_H
