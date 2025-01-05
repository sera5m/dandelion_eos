//improvements to do: iso 8601 parsingg/formatting
//improve the human readable time
//support for by country dst (in progress)



#ifndef mdl_timeKeeping_H
#define mdl_timeKeeping_H

#include <time.h>
#include <sys/time.h>
#include <chrono> //i hope the compiler takes c++ v11
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

// Define weekdays and weekends
const bool isWeekday[7] = { false,true, true, true, true, true, false }; // Mon-Fri: true; Sat, Sun: false (week start with sun, end w sat)


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
struct FriendlyTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
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
    NowUnixTime = now.tv_sec;

    // Store the current Unix time in NVS
    preferences.putULong("unix_time", NowUnixTime);
    Serial.println("Time updated in NVS");
}

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







//*****************************************************************************************************
//clock features
//*****************************************************************************************************


//todo: this needs mercury interaction, and visuals.
//visuals will be made after The visual module revamp, because this module will require popups, an actual window with canvas, and the better formatting for the stopwatch








//**********************************************************************************************************************************************
//alarm features
//(repeating timers idk)
//********************************************************************************************************************************************




struct CustomAlarm {
    bool isEnabled = false;
    FriendlyTime time;
    uint8_t activeDays = 0b00000000; // 8-bit bitmask for days
    int loudness = 50; // Default 50%
    bool flashLed = false;
    bool flashScreen = false;
    bool notifyConnectedDevice = false;
    bool allowSnooze = true;
    int snoozeDuration = 5; // Default 5 minutes
};

//tip: how the bitmask works
/*
Bit 0 (LSB) -> Sunday
Bit 1       -> Monday
Bit 2       -> Tuesday
Bit 3       -> Wednesday
Bit 4       -> Thursday
Bit 5       -> Friday
Bit 6       -> Saturday
Bit 7 (MSB) -> not used. reserved for anything in refactor
*/

//user provides a boolean array (length 7) of days when sending input. [days sun to sat]


// Convert a bitmask to a vector<bool> representing active days
std::vector<bool> DayBitMaskToWeekdays(uint8_t bitmask) {
    std::vector<bool> activeDays(7, false); // Initialize a vector of size 7 with all false

    for (int day = sun; day <= sat; day++) { // Loop through all days
        activeDays[day] = bitmask & (1 << day);
    }
    return activeDays;
}

uint8_t WeekdaysToDayBitMask(const std::vector<bool>& activeDays) {
    if (activeDays.size() != 7) {
        throw std::invalid_argument("Active days vector must have exactly 7 elements.");
    }

    uint8_t bitmask = 0;

    for (int day = 0; day < 7; day++) {
        if (activeDays[day]) {
            bitmask |= (1 << day); // Set the corresponding bit for each active day
        }
    }
    return bitmask;
}

// Utility functions for handling the activeDays bitmask
void setDayActive(uint8_t &bitmask, int day, bool active) {
    if (active) {
        bitmask |= (1 << day); // Set the bit for the given day
    } else {
        bitmask &= ~(1 << day); // Clear the bit for the given day
    }
}

bool isDayActive(uint8_t bitmask, int day) {
    return bitmask & (1 << day); // Check if the bit for the given day is set
}

// Updated isAlarmTime function
bool isAlarmTime(const CustomAlarm &userAlarm, int currentDay, int currentHour, int currentMinute) {
    if (!userAlarm.isEnabled) return false;
    if (!isDayActive(userAlarm.activeDays, currentDay)) return false;

    return (userAlarm.time.hour == currentHour && userAlarm.time.minute == currentMinute);
}


// FreeRTOS task function for CustomAlarm
void alarmTask(void *parameter) {
    CustomAlarm *userAlarm = (CustomAlarm *)parameter; // Cast the parameter to CustomAlarm*

    // Perform alarm actions using the instance
    // // Trigger the alarm actions
    //TODO: ADD THE ALARM TRIGGER BACK IN! HOLY CRAP! I'M just removing it so this compiles

    // Delete the task when done
    vTaskDelete(NULL);
}


// Stack size for the alarm task
constexpr uint16_t AlarmAssignBytes = 512; 

void checkAlarmsWithTask(CustomAlarm *alarms, int numAlarms, int currentDay, int currentHour, int currentMinute) {
    for (int i = 0; i < numAlarms; i++) {
        if (isAlarmTime(alarms[i], currentDay, currentHour, currentMinute)) {
            BaseType_t result = xTaskCreate(alarmTask, "AlarmTask", AlarmAssignBytes, &alarms[i], tskIDLE_PRIORITY, NULL);
            if (result != pdPASS) {
                // Handle task creation failure
                Serial.println("Failed to create AlarmTask");
            }
        }
    }
}

// CustomAlarm NVS storage functions
void saveAlarmsToNVS(CustomAlarm *alarms, int numAlarms) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("alarm_storage", NVS_READWRITE, &nvsHandle);
    if (err == ESP_OK) {
        for (int i = 0; i < numAlarms; i++) {
            char key[16];
            snprintf(key, sizeof(key), "alarm_%d", i); // Create unique key
            nvs_set_blob(nvsHandle, key, &alarms[i], sizeof(CustomAlarm));
        }
        nvs_commit(nvsHandle);
        nvs_close(nvsHandle);
    }
}

int loadAlarmsFromNVS(CustomAlarm *alarms, int maxAlarms) {
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("alarm_storage", NVS_READONLY, &nvsHandle);
    int alarmCount = 0;

    if (err == ESP_OK) {
        for (int i = 0; i < maxAlarms; i++) {
            char key[16];
            snprintf(key, sizeof(key), "alarm_%d", i);

            size_t alarmSize = sizeof(CustomAlarm);
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

// Trigger CustomAlarm
void triggerAlarm(CustomAlarm *userAlarm) {
    if (userAlarm->flashLed) {
        // Flash the LED
    }
    if (userAlarm->flashScreen) {
        // Flash the screen
    }
    if (userAlarm->notifyConnectedDevice) {
        // Push notification
    }
    // Play alarm sound at loudness
}










//TIMER: a non repeating alarm that happens once and then deletes itself

//****************************************************************************************************************************************************************************************************************************
//timer functionality
//****************************************************************************************************************************************************************************************************************************

// Callback function triggered when the timer finishes
void TimerFinish() {
    Serial.println("Timer finished!");
}





class Timer {


private: //private******************************************************

    Preferences preferences;
    String timerName; //TODO: USE CHAR INSTERAD OF STRING

    unsigned long startTime = 0;     // Time when the timer started
    unsigned long duration = 0;      // Original duration in milliseconds
    unsigned long remainingTime = 0; // Time left when paused or modified
    bool running = false;
    bool paused = false;

    esp_timer_handle_t timerHandle = nullptr;

void saveToNVS() {
    preferences.begin("timers", false);
    preferences.putULong((timerName + "_remaining").c_str(), remainingTime);
    preferences.putULong((timerName + "_duration").c_str(), duration);
    preferences.putBool((timerName + "_running").c_str(), running);
    preferences.putBool((timerName + "_paused").c_str(), paused);
    preferences.end();
}

void loadFromNVS() {
    preferences.begin("timers", false);
    remainingTime = preferences.getULong((timerName + "_remaining").c_str(), 0);
    duration = preferences.getULong((timerName + "_duration").c_str(), 0);
    running = preferences.getBool((timerName + "_running").c_str(), false);
    paused = preferences.getBool((timerName + "_paused").c_str(), false);
    preferences.end();
}

void clearFromNVS() {
    preferences.begin("timers", false);
    String key = timerName + "_remaining";
    preferences.remove(key.c_str());
    key = timerName + "_duration";
    preferences.remove(key.c_str());
    key = timerName + "_running";
    preferences.remove(key.c_str());
    key = timerName + "_paused";
    preferences.remove(key.c_str());
    preferences.end();
}

    void setupTimer(unsigned long ms) {
        if (timerHandle != nullptr) {
            esp_timer_stop(timerHandle);
            esp_timer_delete(timerHandle);
        }

        esp_timer_create_args_t timerArgs = {
            .callback = [](void* arg) {
                static_cast<Timer*>(arg)->onTimerFinish(); //cast to the timer on it finishing
            },
            .arg = this, //pointer for args,default void but context aware?
            .name = "user_timer" //default name
        };
        esp_timer_create(&timerArgs, &timerHandle);
        esp_timer_start_once(timerHandle, ms * 1000); // Set timer in microseconds
    }

    void onTimerFinish() {
        running = false;
        paused = false;
        clearFromNVS();
        TimerFinish(); // Trigger the callback
    }


//public**********************************************
public:
    Timer(const String& name) : timerName(name) {
        loadFromNVS();
    }

    ~Timer() {
        if (timerHandle != nullptr) {
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
        saveToNVS();
        setupTimer(duration);
    }

    void stop() {
        if (timerHandle != nullptr) {
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
            remainingTime = remainingTime - (millis() - startTime);
            paused = true;
            running = false;
            if (timerHandle != nullptr) {
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
            if (running) {
                setupTimer(remainingTime);
            }
        }
    }

    void subtractTime(unsigned long seconds) {
        if (paused || running) {
            remainingTime = max(remainingTime, seconds * 1000);
            saveToNVS();
            if (running) {
                setupTimer(remainingTime);
            }
        }
    }

  unsigned long getSecondsLeft() {
        if (running) {
            return max((remainingTime - (millis() - startTime)) / 1000, 0UL);
        }
        return remainingTime / 1000;
    }

    unsigned long getOriginalDuration() {
        return duration / 1000;
    }

    bool isRunning() {
        return running;
    }
};






//to use this, do something like this:

/*

Timer myTimer("timer name here"); //declare an object


    // Start a 10-second timer
    myTimer.start(10); //start the timer

*/







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


#endif // MDL_TIMEKEEPING_H
