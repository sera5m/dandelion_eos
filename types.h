//ligma
#pragma once
#include <cstdint>
//pragma twice would be funny lol

//math
struct int16vect {
    int16_t x;
    int16_t y;
    int16_t z;
    
    // Constructors (now properly inside the struct definition)
    int16vect() : x(0), y(0), z(0) {}
    int16vect(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}
};

//application info




typedef enum{WM_MAIN, WM_STOPWATCH,WM_ALARMS,WM_TIMER,WM_NTP_SYNCH, WM_SET_TIME,WM_SET_TIMEZONE,WM_APPMENU,WM_COUNT}WatchMode;

typedef enum{HMM_BIOMONITOR, //Current fuckshit like that beep boop beeip in hopital
HMM_DAYHISTORY,    //a bar graph over the past x days.
   HMM_HISTORY, //this month/historical trends. on long scales of time we'll just store average hr as waking/sleeping 
   HMM_SETTINGS   //idk man what do you even config here?
}HealthmonitorMode;

typedef enum {
    EDIT_OFF,        // Normal display mode
    EDIT_RUNNING,    // Actively editing values
    EDIT_CONFIRM     // Save/cancel prompt
} EditState;

typedef enum {
    GSLC_POWER,         // sleep modes
    GSLC_ALERTS,        // notifications, alarms
    GSLC_DISPLAY,       // screen settings
    GSLC_DATA,          // storage, sd card
    GSLC_WIRELESS,      // wifi, bt
    GSLC_EXT_HARDWARE,  // modules, sensors
    GSLC_CATEGORY_COUNT
} GlobalSettingsListCategory;

typedef enum {
    PWR_ULTRA,
    PWR_AGGRESSIVE,
    PWR_HEAVY,
    PWR_MODERATE,
    PWR_LIGHT,
    PWR_NONE
} PowerMode;

typedef enum {
    ALERT_SRC_PHONE,
    ALERT_SRC_MISC_INTERNAL,
    ALERT_SRC_CLOCK
} AlertSource;

const char* GlobalSettingsListCategoryNames[] = {
    "power",        // sleep modes
    "alerts",       // notifications, alarms
    "display",      // screen settings
    "data",         // storage, sd card
    "wireless",     // wifi, bt
    "ext_hardware"  // modules, sensors
};


typedef struct {
PowerMode mode;
} PowerSettings;

typedef struct {
    uint8_t intensity;  // 0-255 maybe
    int flash_light;    // bool
    AlertSource source;
} AlertsSettings;

typedef struct {
    uint8_t brightness; // 0-255
    int greyscale;      // bool
    int fast_refresh;   // bool
} DisplaySettings;

typedef struct {
    int nfc_enabled;  // bool
    int wifi_enabled; // bool
    int bt_enabled;   // bool
} WirelessSettings;

typedef struct {
    uint32_t storage_used_mb;
    uint32_t storage_total_mb;
} DataSettings;
typedef struct {
    PowerSettings power;
    AlertsSettings alerts;
    DisplaySettings display;
    DataSettings data;
    WirelessSettings wireless;
    // ext_hardware can be fleshed out later
} GlobalSettings;


const char* appNames[] = {
  "lock screen",
  "health",
  "NFC Tools",
  "Settings",
  "Gyro Info",
  "Files",
  "Radio",
  "IR Remote",
  "utilities",
  "eTools", // combination of oscilloscope and signal gen
  "rubberducky",
  "connections",
  "smart devices",
  "Diagnostics"
};

typedef enum {
  APP_LOCK_SCREEN,
  APP_HEALTH,
  APP_NFC_TOOLS,
  APP_SETTINGS,
  APP_GYRO_INFO,
  APP_FILES,
  APP_RADIO,
  APP_IR_REMOTE,
  APP_UTILITIES,
  APP_ETOOLS,
  APP_RUBBERDUCKY,
  APP_CONNECTIONS,
  APP_SMART_DEVICES,
  APP_DIAGNOSTICS,
  APP_COUNT // always handy for bounds checking
}AppName;


