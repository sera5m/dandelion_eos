#include "MDL_CLOCK.h"

//improvements to do: iso 8601 parsingg/formatting
//improve the human readable time
//support for by country dst (in progress)

#include <pgmspace.h>
#include "esp_sntp.h"


#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <optional>
// Global variables for time access
#include "timezones.h" //source: don't even ask
#include <Wire.h>
#include "driver/timer.h"
// -IN main not here
//int currentHour = 0;
//int currentMinute = 0;
//int currentSecond = 0;
#include <stdio.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "Wiring.h"





const char* TRIchar_month_names[] = {
    "INVALID", // index 0
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#include <Adafruit_NeoPixel.h>
#include "mdl_clock.h"

const char* DayNames[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char* AlarmActionNames[] = {"NONE", "LIGHT", "buzz", "both", "phone"};
//const uint8_t ALARM_ACTION_MAX = sizeof(AlarmActionNames)/sizeof(AlarmActionNames[0]);
/*
void syncTimeFromNTP(const char* ntpServer) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, ntpServer);
    sntp_init();
}
*/

void updateCurrentTimeVars() {
    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_r(&now, &tm_now);

    CurrentNormieTime.year   = tm_now.tm_year - 100;  // tm_year = years since 1900
    CurrentNormieTime.month  = tm_now.tm_mon + 1;     // tm_mon = 0–11 → make it 1–12
    CurrentNormieTime.day    = tm_now.tm_mday;
    CurrentNormieTime.hour   = tm_now.tm_hour;
    CurrentNormieTime.minute = tm_now.tm_min;
    CurrentNormieTime.second = tm_now.tm_sec;
}

//const bool isWeekday[7] = { false, true, true, true, true, true, false };

NormieTime convertToNormieTime(time_t t) {
    struct tm tm_struct;
    localtime_r(&t, &tm_struct);
    return {
        uint8_t(tm_struct.tm_year - 100),
        uint8_t(tm_struct.tm_mon),
        uint8_t(tm_struct.tm_mday - 1),
        uint8_t(tm_struct.tm_hour),
        uint8_t(tm_struct.tm_min),
        uint8_t(tm_struct.tm_sec)
    };
}

// Parse a timezone rule string into TimezoneInfo
void parseTimezoneRule(const char* rule, TimezoneInfo* info) {
    memset(info, 0, sizeof(*info));
    if (!strchr(rule, ',')) {
        info->has_dst = false;
        parseSimpleOffset(rule, info);
        return;
    }
    info->has_dst = true;
    parseDSTRule(rule, info);
}

bool isLeapYear(int year) {
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

// Helper for simple offsets (no DST)
void parseSimpleOffset(const char* rule, TimezoneInfo* info) {
    const char* p = rule;
    while (*p && !isdigit(*p) && *p != '-' && *p != '+') p++;
    size_t len = p - rule;
    if (len < sizeof(info->std_abbr)) {
        memcpy(info->std_abbr, rule, len);
        info->std_abbr[len] = '\0';
    }
    float hrs = atof(p);
    info->standard_offset = int8_t(hrs * 4);
    info->dst_offset = info->standard_offset;
}

void parseDSTRule(const char* rule, TimezoneInfo* info) {
    char buf[64];
    strncpy(buf, rule, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    char* stdp = strtok(buf, ",");
    char* stp  = strtok(nullptr, ",");
    char* edp  = strtok(nullptr, ",");
    if (!stdp || !stp || !edp) return;
    parseStandardPart(stdp, info);
    parseTransitionRule(stp, &info->start_dst);
    parseTransitionRule(edp, &info->end_dst);
}

void parseStandardPart(const char* part, TimezoneInfo* info) {
    const char* p = part;
    while (*p && isalpha(*p)) p++;
    size_t len = p - part;
    if (len < sizeof(info->std_abbr)) {
        memcpy(info->std_abbr, part, len);
        info->std_abbr[len] = '\0';
    }
    info->standard_offset = int8_t(atof(p) * 4);
    while (*p && !isalpha(*p)) p++;
    if (*p) {
        strncpy(info->dst_abbr, p, sizeof(info->dst_abbr)-1);
        info->dst_offset = info->standard_offset + 4;
    }
}

void parseTransitionRule(const char* rule, TimezoneTransitionDate* d) {
    if (rule[0] != 'M') return;
    int m, w, day, hr=0;
    // sscanf is simpler here
    if (sscanf(rule, "M%d.%d.%d/%d", &m, &w, &day, &hr) >= 3) {
        d->month = m - 1;
        d->week  = w;
        d->day   = day;
        d->hour  = hr;
    }
}

void initialTimezoneSetup() {
    const zones_t zone = {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"};
    TimezoneInfo info;
    parseTimezoneRule( zone.zones.c_str(), &info);
    //Serial.printf("Timezone: %s\n", zone.name);
    //Serial.printf("STD: %s (UTC%+.2f)\n", info.std_abbr, info.standard_offset/4.0f);
    if (info.has_dst) {
        /*
        Serial.printf("DST: %s (UTC%+.2f)\n", info.dst_abbr, info.dst_offset/4.0f);
        Serial.printf("Starts: %d.%d wk%d@%d\n",
                      info.start_dst.month+1, info.start_dst.day+1,
                      info.start_dst.week, info.start_dst.hour);
        Serial.printf("Ends:   %d.%d wk%d@%d\n\n",
                      info.end_dst.month+1, info.end_dst.day+1,
                      info.end_dst.week, info.end_dst.hour);
                      */
    }
}
extern Adafruit_NeoPixel strip;


void blink_led(uint32_t color, uint8_t times, uint16_t delay_ms) {
  for (int i = 0; i < times; i++) {
    strip.setPixelColor(0, color); // Turn ON with color
    strip.show();
    delay(delay_ms);

    strip.setPixelColor(0, 0); // Turn OFF
    strip.show();
    delay(delay_ms);
  }
}

uint64_t get_time_until_timer_us(usr_alarm_st* alarm) {
    struct timeval now;
    gettimeofday(&now, NULL);
    // Convert current time to minutes since midnight
    int now_minutes = (now.tv_sec / 60) % (24 * 60);
    int alarm_minutes = alarm->hours * 60 + alarm->minutes;

    int diff = alarm_minutes - now_minutes;
    if (diff <= 0) {
        diff += 24 * 60; // next day
    }

    return (uint64_t)diff * 60 * 1000000ULL; // microseconds
}

void make_timer(usr_alarm_st* alarm) {
    uint64_t sleep_time_us = get_time_until_alarm_us(alarm);

    ESP_LOGI(TAG, "Sleeping for %" PRIu64 " us (~%lld minutes)", sleep_time_us, sleep_time_us / 60000000ULL);

    esp_sleep_enable_timer_wakeup(sleep_time_us);
    esp_deep_sleep_start();

    // ESP32 restarts here on wakeup, so code below won't run after deep sleep
}

const char* alarmActionToString(alarmAction act) {
  switch (act) {
    case light: return "Light";
    case buzzer: return "Buzzer";
    case phonebuzzer: return "PhoneBuzz";
    case both: return "Both";
    default: return "Unknown";
  }
}

void usr_alarm_st_to_str(const usr_alarm_st *alarm, char *outBuf, size_t bufSize) {
  uint8_t r = (alarm->LightColor >> 16) & 0xFF;
  uint8_t g = (alarm->LightColor >> 8) & 0xFF;
  uint8_t b = alarm->LightColor & 0xFF;

  uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

  snprintf(outBuf, bufSize,  // <-- the *destination*
    "<textsize(2)>%02u:%02u<n><textsize(1)>%s<n><textcolor(0x%04X)>",
    alarm->hours,
    alarm->minutes,
    alarmActionToString(alarm->E_AlarmAction),
    rgb565);
}

usr_alarm_st usrmade_alarms[10]; 
usr_alarm_st usrmade_timers[5];


/*
// Attach to RTC or system clock
void setupTimeEvents() {
    // Example: Use ESP32 timer (adjust for your hardware)
    esp_timer_create_args_t timerArgs = {
        .callback = [](void* arg) { onMinuteTick(); },
        .arg = nullptr,
        .name = "minute_tick"
    };
    esp_timer_handle_t timerHandle;
    esp_timer_create(&timerArgs, &timerHandle);
    esp_timer_start_periodic(timerHandle, 60 * 1000000); // 60 seconds
}



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

/// Update the stored Unix time in NVS
//should get graceful faulure handling later

// Print the current local time
void GetLocalTime() {
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

// Convert Unix time to local NormieTime with time zone support
void UnixTimeToNormieTime(time_t unixTime, int timeZoneOffset, NormieTime &ft) {
    struct tm timeinfo;
    gmtime_r(&unixTime, &timeinfo); // Converts Unix time to UTC

    timeinfo.tm_hour += timeZoneOffset; // Adjust for time zone
    mktime(&timeinfo); // Normalize the time structure

    ft.year = timeinfo.tm_year + 1900;
    ft.month = timeinfo.tm_mon + 1;
    ft.day = timeinfo.tm_mday;
    ft.hour = timeinfo.tm_hour;
    ft.minute = timeinfo.tm_min;
    ft.second = timeinfo.tm_sec;
}

//tip: use above to convert to time and date from unix ticks, or convert unix tick to the normal time. i think. 

//use this function to convert seconds (say from a timer) to normative time structures. uses mod to provide ms as a remainder
//idk how well this works but it should work fine, ig?
// Function to convert raw seconds to friendly time
void RawSecondsToNormieTime(float seconds, NormieTime &NormieTime, float &ms) {
    // Extract the fractional part (milliseconds) and the whole number part (seconds)
    float fractionalPart = 0.0f;
    float wholeSeconds = std::modf(seconds, &fractionalPart);
    
    // Convert fractional part to milliseconds
    ms = fractionalPart * 1000;

    // Convert whole seconds into integer
    int totalSeconds = static_cast<int>(wholeSeconds);

    // 1. Convert seconds to minutes, then to hours, days, and years
    NormieTime.second = totalSeconds % 60;
    totalSeconds /= 60;  // Remaining seconds after getting the minute

    if (totalSeconds > 0) {
        NormieTime.minute = totalSeconds % 60;
        totalSeconds /= 60;  // Remaining after minutes to hours
    } else {
        NormieTime.minute = 0;
    }

    if (totalSeconds > 0) {
        NormieTime.hour = totalSeconds % 24;
        totalSeconds /= 24;  // Remaining after hours to days
    } else {
        NormieTime.hour = 0;
    }

    if (totalSeconds > 0) {
        NormieTime.day = totalSeconds % 365;  // Assuming 365 days per year
        totalSeconds /= 365;  // Remaining after days to years
    } else {
    
        NormieTime.day = 0;
    }

    // Calculate the year (assuming 365.24 days per year for simplicity)
    NormieTime.year = 1970 + totalSeconds;

    // Month calculation (just an approximation based on days in the year)
    // Assume 12 months, divide days by 30 for rough month estimate
    NormieTime.month = (NormieTime.day / 30) + 1;
    if (NormieTime.month > 12) {
        NormieTime.month = 12;
    }

    // The day is now adjusted for the month, subtract full months
    NormieTime.day = NormieTime.day % 30;
}

/*
// Convert NormieTime to Unix time with time zone adjustment
time_t NormieTimeToUnixTime(const NormieTime &NormieTime, int timeZoneOffset) {
    struct tm timeinfo = {};
    timeinfo.tm_year = NormieTime.year - 1900;
    timeinfo.tm_mon = NormieTime.month - 1;
    timeinfo.tm_mday = NormieTime.day;
    timeinfo.tm_hour = NormieTime.hour;
    timeinfo.tm_min = NormieTime.minute;
    timeinfo.tm_sec = NormieTime.second;

    // Convert to Unix time and adjust for time zone offset
    return mktime(&timeinfo) - timeZoneOffset;
}

// Provide normalized local time (NormieTime struct) from Unix time
void GetNormieTime(time_t unixTime, int timeZoneOffset, NormieTime &NormieTime) {
    UnixTimeToNormieTime(unixTime, timeZoneOffset, NormieTime);
}

NormieTime TimeUntil(const NormieTime &current, const NormieTime &target) { // time a to time b, friendly time
    time_t currentUnix = NormieTimeToUnixTime(current, UserTimeZoneOffset); // Corrected variable
    time_t targetUnix = NormieTimeToUnixTime(target, UserTimeZoneOffset);

    time_t diff = targetUnix - currentUnix;
    NormieTime result;
    UnixTimeToNormieTime(diff, 0, result); // Offset = 0 since this is a duration
    return result;
}



*/



//*****************************************************************************************************
//clock features
//*****************************************************************************************************


//todo: this needs mercury interaction, and visuals.
//visuals will be made after The visual module revamp, because this module will require popups, an actual window with canvas, and the better formatting for the stopwatch








//**********************************************************************************************************************************************
//alarm features
//alarms repeat on a daily basis
//********************************************************************************************************************************************


/*

#pragma pack(push, 1) // Exact byte packing
typedef struct {
    uint8_t enabled : 1;
    uint8_t hour : 5;    // 0-23 (5 bits)
    uint8_t minute : 6;  // 0-59 (6 bits, rounded to 5min)
    uint8_t days : 7;    // Bitmask (Sun-Sat)
    uint8_t actions : 3; // Bitmask: 0x1=LED, 0x2=Screen, 0x4=Buzz
    uint8_t snoozeDur : 1; //time that alarm may be snoozed for. use 0 for no snoozes
} CoProcAlarm;
#pragma pack(pop)

// Main CPU executes this hourly to load any alarms to the co-proscessor so alarms run while the device sleepys
void load_hourly_alarms(int current_hour) {
    // 1. Load ALL alarms from NVS (once at boot)
    static CoProcAlarm all_alarms[MAX_ALARMS];
    static bool loaded = false;
    if(!loaded) {
        load_from_nvs("alarms", all_alarms, sizeof(all_alarms));
        loaded = true;
    }

    // 2. Filter alarms for current hour
    CoProcAlarm hour_alarms[MAX_ALARMS];
    uint8_t count = 0;
    for(int i=0; i<MAX_ALARMS && count<MAX_ALARMS; i++) {
        if(all_alarms[i].enabled && all_alarms[i].hour == current_hour) {
            hour_alarms[count++] = all_alarms[i];
        }
    }

    // 3. Copy to co-processor shared RTC memory
    memcpy(RTC_SLOW_MEM + ALARM_STORAGE_OFFSET, hour_alarms, count*sizeof(CoProcAlarm));
    *((uint8_t*)(RTC_SLOW_MEM + ALARM_COUNT_OFFSET)) = count;
}

// Shared RTC memory layout
typedef struct {
    uint8_t alarm_count;
    CoProcAlarm alarms[10];
    uint32_t checksum;
} RtcAlarmBlock;

// Initialize co-processor
void init_alarm_coproc() {
    // 1. Reserve RTC memory
    esp_err_t err = rtc_slow_mem_alloc(sizeof(RtcAlarmBlock));
    
    // 2. Load ULP program
    ulp_process_macros_and_load(ulp_alarm_bin, RTC_SLOW_MEM + ULP_CODE_OFFSET);
    
    // 3. Set hourly update timer
    esp_timer_create(...hourly_update...);
}

// Hourly update callback
void hourly_update() {
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    
    // 1. Load alarms for this hour
    load_hourly_alarms(tm->tm_hour);
    
    // 2. Update RTC time (4-byte unix timestamp)
    *(uint32_t*)(RTC_SLOW_MEM + TIME_OFFSET) = (uint32_t)now;
    
    // 3. Verify checksum
    update_rtc_checksum();
}






//TIMER: a non repeating alarm that happens once and then deletes itself

//****************************************************************************************************************************************************************************************************************************
//timer functionality
//****************************************************************************************************************************************************************************************************************************

// Callback function triggered when the timer finishes
void TimerFinish() {
    Serial.println("Timer finished!");
}






// Timer class
class Timer {
private:
    Preferences preferences;
    char timerName[32]; // Replacing String with char[] to reduce heap fragmentation

    unsigned long startTime = 0;
    unsigned long duration = 0;
    unsigned long remainingTime = 0;
    bool running = false;
    bool paused = false;

    esp_timer_handle_t timerHandle = nullptr;

    void saveToNVS() {
        preferences.begin("timers", false);
        preferences.putULong((String(timerName) + "_remaining").c_str(), remainingTime);
        preferences.putULong((String(timerName) + "_duration").c_str(), duration);
        preferences.putBool((String(timerName) + "_running").c_str(), running);
        preferences.putBool((String(timerName) + "_paused").c_str(), paused);
        preferences.end();
    }

    void loadFromNVS() {
        preferences.begin("timers", false);
        remainingTime = preferences.getULong((String(timerName) + "_remaining").c_str(), 0);
        duration = preferences.getULong((String(timerName) + "_duration").c_str(), 0);
        running = preferences.getBool((String(timerName) + "_running").c_str(), false);
        paused = preferences.getBool((String(timerName) + "_paused").c_str(), false);
        preferences.end();
    }

    void clearFromNVS() {
        preferences.begin("timers", false);
        preferences.remove((String(timerName) + "_remaining").c_str());
        preferences.remove((String(timerName) + "_duration").c_str());
        preferences.remove((String(timerName) + "_running").c_str());
        preferences.remove((String(timerName) + "_paused").c_str());
        preferences.end();
    }

    void setupTimer(unsigned long ms) {
        if (timerHandle) {
            esp_timer_stop(timerHandle);
            esp_timer_delete(timerHandle);
        }

        esp_timer_create_args_t timerArgs = {
            .callback = [](void* arg) { static_cast<Timer*>(arg)->onTimerFinish(); },
            .arg = this,
            .name = "user_timer"
        };
        esp_timer_create(&timerArgs, &timerHandle);
        esp_timer_start_once(timerHandle, ms * 1000);
    }

    void onTimerFinish() {
        running = false;
        paused = false;
        clearFromNVS();
        TimerFinish();
    }

public:
    Timer(const char* name) {
        strncpy(timerName, name, sizeof(timerName) - 1);
        timerName[sizeof(timerName) - 1] = '\0';
        loadFromNVS();
    }

    ~Timer() {
        if (timerHandle) {
            esp_timer_stop(timerHandle);
            esp_timer_delete(timerHandle);
        }
        saveToNVS();
    }

    void start(unsigned long durationInSeconds) {
        duration = durationInSeconds * 1000;
        remainingTime = duration;
        running = true;
        paused = false;
        startTime = millis();
        saveToNVS();
        setupTimer(duration);
    }

    void stop() {
        if (timerHandle) {
            esp_timer_stop(timerHandle);
            esp_timer_delete(timerHandle);
            timerHandle = nullptr;
        }
        running = false;
        paused = false;
        clearFromNVS();
    }

    void pause() {
        if (running && !paused) {
            remainingTime = max(remainingTime - (millis() - startTime), 0UL);
            paused = true;
            running = false;
            if (timerHandle) {
                esp_timer_stop(timerHandle);
                esp_timer_delete(timerHandle);
                timerHandle = nullptr;
            }
            saveToNVS();
        }
    }

    void resume() {
        if (paused) {
            running = true;
            paused = false;
            startTime = millis();
            saveToNVS();
            setupTimer(remainingTime);
        }
    }

    void addTime(unsigned long seconds) {
        if (paused || running) {
            remainingTime += seconds * 1000;
            saveToNVS();
            if (running) setupTimer(remainingTime);
        }
    }

    void subtractTime(unsigned long seconds) {
        if (paused || running) {
            remainingTime = (remainingTime > (seconds * 1000)) ? (remainingTime - (seconds * 1000)) : 0;
            saveToNVS();
            if (running) setupTimer(remainingTime);
        }
    }

    unsigned long getSecondsLeft() {
        return running ? max((remainingTime - (millis() - startTime)) / 1000, 0UL) : remainingTime / 1000;
    }

    unsigned long getOriginalDuration() { return duration / 1000; }
    bool isRunning() { return running; }
};



//to use this, do something like this:



Timer myTimer("timer name here"); //declare an object


    // Start a 10-second timer
    myTimer.start(10); //start the timer







*/

//***********************************************************************************************************************************************************************************************************************************************************************
//stopwatch functionality

//************************************************

//std chrono 


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
       /* Serial.println("No laps recorded.");*/
        return;
    }
    for (size_t i = 0; i < laps.size(); ++i) {
     /*   Serial.print("Lap ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(laps[i], 4); // Print with 4 decimal places
        Serial.println(" seconds");
        */
    }
}


}; //end stopwatch obj

//to use the stopwatch:

//create object: Stopwatch [name];
//start timer: [object name].start();
//record laps: [object name].recordLap();
//stop timer: [object name].stop();
//print laps: [object name].printLaps();



//

//end the module



