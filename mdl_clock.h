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
#include "driver/timer.h"
#include <stdio.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "Wiring.h"
static const char* TAG = "alarm";
#include <Adafruit_NeoPixel.h>


// ========== ENUMS ========== //

enum days { sun, mon, tue, wed, thu, fri, sat };
extern const char* DayNames[];
extern const char* AlarmActionNames[];

enum months { jan, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec };
extern const char* TRIchar_month_names[13];

typedef enum{none,light,buzzer,both,phonebuzzer,ALARM_ACTION_MAX}alarmAction;
//light blink,buzzer, a text, or blink and buzz
extern const char* AlarmActionNames[5];

// ========== STRUCTS ========== //



typedef struct __attribute__((packed)){
uint8_t hours; //2 digits
uint8_t minutes; //2 digits
alarmAction E_AlarmAction;
uint32_t LightColor; //heads up it's 24 bits for color,the days is packed in the last 8
uint8_t SnoozeDur; //1 field
}usr_alarm_st;//todo pack or this may be unreliable

const uint8_t NUM_TIMER_FIELDS = 16; //this is almost certainly wrong



//works for both alarm and timer-but for alarms you may have to add in the days map for weekdays vs weekend.
// note that the color bitmask is 32x but stores 24 bits of color..... we will utilize the last 8 bits to store THE DAYS ACTIVE BITMASK in the future-8 bits
//done by fn splitu32_to24

extern usr_alarm_st usrmade_alarms[10]; 
extern usr_alarm_st usrmade_timers[5];
//yes, these share the same data type, because it's a reusable generic

struct NormieTime {
    uint16_t year=2025;   // offset from 2000 AD
    uint8_t month=6;
    uint8_t day=15;
    uint8_t hour=23;
    uint8_t minute=15;
    uint8_t second=54;
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

// ========== GLOBAL VARS ========== //
extern int currentHour;
extern int currentMinute;
extern int currentSecond;
extern NormieTime CurrentNormieTime;

// ========== CONSTANTS ========== //
const bool isWeekday[7] = { false, true, true, true, true, true, false };





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
void init_led_flashlight();
void blink_led(uint32_t color, uint8_t times);
uint64_t get_time_until_alarm_us(usr_alarm_st* alarm);
void make_alarm(usr_alarm_st* alarm);

const char* alarmActionToString(alarmAction act);
void usr_alarm_st_to_str(const usr_alarm_st *alarm, char *outBuf, size_t bufSize);

#endif // MDL_CLOCK_H
