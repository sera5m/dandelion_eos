#ifndef MDL_CLOCK_H
#define MDL_CLOCK_H


#include <pgmspace.h>
#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <optional>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include "timezones.h"
#include <Wire.h>
// ========== ENUMS ========== //

enum days  { sun, mon, tue, wed, thu, fri, sat };
enum months{ jan, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec };

enum AppScreenState{ //mode the watch is in(well, what screen you're on, stuff will run in the background)
disabled,background,settings,alarms,stopwatch,timers,cycle
};
// ========== GLOBAL VARS ========== //
extern int currentHour;
extern int currentMinute;
extern int currentSecond;

// ========== CONSTANTS ========== //
const bool isWeekday[7] = { false, true, true, true, true, true, false };

// ========== STRUCTS ========== //



struct NormieTime {
    uint8_t year;   // offset from 2000 AD
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct TimezoneTransitionDate {
    int8_t month;
    int8_t week;
    int8_t day;   // 0=Sun, 6=Sat
    int8_t hour;
};

struct TimezoneInfo {
    int8_t standard_offset;   // In 15-min intervals (e.g., UTC-8 = -32)
    int8_t dst_offset;
    char std_abbr[6];         // "PST", "CET", etc
    char dst_abbr[6];         // "PDT", "CEST", etc
    TimezoneTransitionDate start_dst;
    TimezoneTransitionDate end_dst;
    bool has_dst;
};

// ========== FUNCTIONS ========== //

// Convert time_t to NormieTime struct
NormieTime convertToNormieTime(time_t t);

// Parse full timezone rule into info
void parseTimezoneRule(const char* rule, TimezoneInfo* info);

// Helpers for timezone parsing
void parseSimpleOffset(const char* rule, TimezoneInfo* info);
void parseDSTRule(const char* rule, TimezoneInfo* info);
void parseStandardPart(const char* part, TimezoneInfo* info);
void parseTransitionRule(const char* rule, TimezoneTransitionDate* date);

// Setup function (to call from setup())
void initialTimezoneSetup();

// NTP sync
//void syncTimeFromNTP(const char* ntpServer);

void updateCurrentTimeVars();


#endif // MDL_CLOCK_H
