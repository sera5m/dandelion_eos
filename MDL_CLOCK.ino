//improvements to do: iso 8601 parsingg/formatting
//improve the human readable time
//support for by country dst (in progress)



#ifndef MDL_CLOCK_H
#define MDL_CLOCK_H

#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <optional>
// Global variables for time access


//default enums and some vars
extern int currentHour;
extern int currentMinute;
extern int currentSecond;

enum days{ 
  sun,
  mon,
  tue,
  wend,
  thur,
  fri,
  sat
};
enum months{
  jan,
  feb,
  mar,
  apr,
  may,
  jun,
  jul,
  aug,
  sept,
  oct,
  nov,
  dec
};

// Define weekdays and weekends
const bool isWeekday[7] = { false,true, true, true, true, true, false }; // Mon-Fri: true; Sat, Sun: false (week start with sun, end w sat)
/*


// Global variable for Unix time
extern time_t NowUnixTime;


//stuff 4 dst
int UserTimeZoneOffset = 0; // your timezone relative to utc+0 IN SECONDS. 
bool useDST=false; //use daylight savings time? (likely needs to be either set or stored in nvs or something)
bool ForceDisableDST=false; //set to true if the user wants to forcibly disable use of dst 

// Enumerate countries (add more as needed)
enum country {
    USA,
    Canada,
    UK,
    Germany,
    Australia,
    Japan,
    India,
    China,
    Brazil,
    SouthAfrica,
    // Add other countries AND REGIONS here!!! THIS LIST IS NOT OCMPLETE
};


enum country userCountry = USA; //default user country

// Structure to hold timezone and DST rules
typedef struct {
    int standardOffset; // Standard offset in seconds from UTC
    bool usesDST;       // Does the country use DST?
    int dstStartMonth;  // Start month for DST
    int dstStartDay;    // Approximate start day (e.g., "2nd Sunday of March")
    int dstEndMonth;    // End month for DST
    int dstEndDay;      // Approximate end day (e.g., "1st Sunday of November")
} TimeZoneInfo;


// Lookup table for countries and their timezone information
// TEMPORARY!!! todo: ADD MORE COUNTRIES IF NEEDED. 
//incomplete timezone coverage for west and east eu and continental USA, south america may need work. check this whole table over

//offset in s, has dst,date of dst start, dst end
const TimeZoneInfo timeZoneTable[] = {
    { -18000, true, mar, 14, nov, 7 },   // USA: UTC-5, DST starts 2nd Sunday of March, ends 1st Sunday of November
    { -18000, true, mar, 14, nov, 7 },   // Canada: Same as USA
    {     0,  true, mar, 28, oct, 31 },  // UK: UTC+0, DST starts last Sunday of March, ends last Sunday of October
    {  3600, true, mar, 28, oct, 31 },  // Germany: UTC+1, same DST as UK
    {  36000, true, oct, 1, apr, 1 },   // Australia: UTC+10, DST varies but generally Oct-Apr
    {  32400, false, 0, 0, 0, 0 },      // Japan: UTC+9, no DST
    {  19800, false, 0, 0, 0, 0 },      // India: UTC+5:30, no DST
    {  28800, false, 0, 0, 0, 0 },      // China: UTC+8, no DST
    { -10800, true, oct, 1, feb, 15 },  // Brazil: UTC-3, DST varies but generally Oct-Feb
    {  7200, false, 0, 0, 0, 0 }        // South Africa: UTC+2, no DST
    // PLEASE Add other countries here
};

// Function to calculate user timezone offset
int calculateUserTimeZoneOffset(enum country userCountry, enum months currentMonth, int day, bool forceDisableDST) {
    const TimeZoneInfo *tzInfo = &timeZoneTable[userCountry];

    // Start with the standard offset
    int offset = tzInfo->standardOffset;

    // Check if DST should be applied
    if (!forceDisableDST && tzInfo->usesDST) {
        if ((currentMonth > tzInfo->dstStartMonth && currentMonth < tzInfo->dstEndMonth) ||
            (currentMonth == tzInfo->dstStartMonth && day >= tzInfo->dstStartDay) ||
            (currentMonth == tzInfo->dstEndMonth && day < tzInfo->dstEndDay)) {
            offset += 3600; // Add one hour for DST
        }
    }

    return offset;
}

*/

//struct for time in a nice human readable format
struct FriendlyTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};


//the folllowing code is to be ticked once per minutes so that allar,/timer whatever the fuck can be ran efficiently without a freerots task

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

// Convert Unix time to local FriendlyTime with time zone support
void UnixTimeToFriendlyTime(time_t unixTime, int timeZoneOffset, FriendlyTime &ft) {
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
void RawSecondsToFriendlyTime(float seconds, FriendlyTime &FriendlyTime, float &ms) {
    // Extract the fractional part (milliseconds) and the whole number part (seconds)
    float fractionalPart = 0.0f;
    float wholeSeconds = std::modf(seconds, &fractionalPart);
    
    // Convert fractional part to milliseconds
    ms = fractionalPart * 1000;

    // Convert whole seconds into integer
    int totalSeconds = static_cast<int>(wholeSeconds);

    // 1. Convert seconds to minutes, then to hours, days, and years
    FriendlyTime.second = totalSeconds % 60;
    totalSeconds /= 60;  // Remaining seconds after getting the minute

    if (totalSeconds > 0) {
        FriendlyTime.minute = totalSeconds % 60;
        totalSeconds /= 60;  // Remaining after minutes to hours
    } else {
        FriendlyTime.minute = 0;
    }

    if (totalSeconds > 0) {
        FriendlyTime.hour = totalSeconds % 24;
        totalSeconds /= 24;  // Remaining after hours to days
    } else {
        FriendlyTime.hour = 0;
    }

    if (totalSeconds > 0) {
        FriendlyTime.day = totalSeconds % 365;  // Assuming 365 days per year
        totalSeconds /= 365;  // Remaining after days to years
    } else {
    
        FriendlyTime.day = 0;
    }

    // Calculate the year (assuming 365.24 days per year for simplicity)
    FriendlyTime.year = 1970 + totalSeconds;

    // Month calculation (just an approximation based on days in the year)
    // Assume 12 months, divide days by 30 for rough month estimate
    FriendlyTime.month = (FriendlyTime.day / 30) + 1;
    if (FriendlyTime.month > 12) {
        FriendlyTime.month = 12;
    }

    // The day is now adjusted for the month, subtract full months
    FriendlyTime.day = FriendlyTime.day % 30;
}

/*
// Convert FriendlyTime to Unix time with time zone adjustment
time_t FriendlyTimeToUnixTime(const FriendlyTime &FriendlyTime, int timeZoneOffset) {
    struct tm timeinfo = {};
    timeinfo.tm_year = FriendlyTime.year - 1900;
    timeinfo.tm_mon = FriendlyTime.month - 1;
    timeinfo.tm_mday = FriendlyTime.day;
    timeinfo.tm_hour = FriendlyTime.hour;
    timeinfo.tm_min = FriendlyTime.minute;
    timeinfo.tm_sec = FriendlyTime.second;

    // Convert to Unix time and adjust for time zone offset
    return mktime(&timeinfo) - timeZoneOffset;
}

// Provide normalized local time (FriendlyTime struct) from Unix time
void GetFriendlyTime(time_t unixTime, int timeZoneOffset, FriendlyTime &FriendlyTime) {
    UnixTimeToFriendlyTime(unixTime, timeZoneOffset, FriendlyTime);
}

FriendlyTime TimeUntil(const FriendlyTime &current, const FriendlyTime &target) { // time a to time b, friendly time
    time_t currentUnix = FriendlyTimeToUnixTime(current, UserTimeZoneOffset); // Corrected variable
    time_t targetUnix = FriendlyTimeToUnixTime(target, UserTimeZoneOffset);

    time_t diff = targetUnix - currentUnix;
    FriendlyTime result;
    UnixTimeToFriendlyTime(diff, 0, result); // Offset = 0 since this is a duration
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









//***********************************************************************************************************************************************************************************************************************************************************************
//stopwatch functionality

//************************************************

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
        Serial.println("No laps recorded.");
        return;
    }
    for (size_t i = 0; i < laps.size(); ++i) {
        Serial.print("Lap ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(laps[i], 4); // Print with 4 decimal places
        Serial.println(" seconds");
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
*/

#endif // MDL_TIMEKEEPING_H
