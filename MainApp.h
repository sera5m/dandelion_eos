#ifndef MAINAPP_H
#define MAINAPP_H
#include <cstdint>
#include "types.h"
#include "Wiring.h"
#include "mdl_clock.h"
#include "helperfunctions.h"
#include "InputHandler.h"
#include "mdl_clock.h"
#include "s_hell.h"
#include "Micro2D_A.h"
#include "globals.h"
#include <cstdint>
#include <Arduino.h>

#pragma once
extern std::unique_ptr<WindowManager> WinManagerInstance;
extern std::shared_ptr<Window> Win_GeneralPurpose; 
extern std::shared_ptr<Window> lockscreen_biomon;
extern std::shared_ptr<Window> lockscreen_thermometer;
extern char thermoStr[8];
extern char hrStr[8];
extern bool IsScreenOn;//why. just why

extern TaskHandle_t watchScreenHandle;
#define MAX_VISIBLE 15
extern char buf_applist[25 * MAX_VISIBLE]; //
extern char watchscreen_buf[WATCHSCREEN_BUF_SIZE];

extern EditState timerEditState;
extern uint8_t currentTimerField;

// Watch mode and timers
extern WatchMode currentWatchMode;
extern int stopwatchElapsed;
extern uint8_t selectedTimerIndex;

// UI state
extern bool is_watch_screen_in_menu;
extern bool isConfirming;

// Stopwatch
extern bool stopwatchRunning;
extern unsigned long stopwatchStart;
extern uint8_t watchModeIndex;
extern uint16_t WatchScreenUpdateInterval;


extern usr_alarm_st usrmade_alarms[10]; 
extern usr_alarm_st usrmade_timers[5];

typedef enum {
    TIMER_FIELD_HOUR_TENS = 0,   // First digit of hour [0-2]
    TIMER_FIELD_HOUR_ONES,       // Second digit of hour [0-9] (0-3 if tens=2)
    TIMER_FIELD_MIN_TENS,        // First digit of minute [0-5]
    TIMER_FIELD_MIN_ONES,        // Second digit of minute [0-9]
    TIMER_FIELD_ALARM_ACTION,    // Alarm type selection
    TIMER_FIELD_SNOOZE,          // Snooze duration [1-30]
    TIMER_FIELD_COUNT            // Total fields
} TimerField;

typedef enum {
    ALARM_FIELD_HOUR_TENS,   // First digit of hour [0-2]
    ALARM_FIELD_HOUR_ONES,       // Second digit of hour [0-9] (0-3 if tens=2)
    ALARM_FIELD_MIN_TENS,        // First digit of minute [0-5]
    ALARM_FIELD_MIN_ONES,        // Second digit of minute [0-9]
    ALARM_FIELD_ALARM_ACTION,    // Alarm type selection
    ALARM_FIELD_SNOOZE,          // Snooze duration [1-30]
    ALARM_FIELD_DAY_MON,         // Day selection starts here
    ALARM_FIELD_DAY_TUE,
    ALARM_FIELD_DAY_WED,
    ALARM_FIELD_DAY_THU,
    ALARM_FIELD_DAY_FRI,
    ALARM_FIELD_DAY_SAT,
    ALARM_FIELD_DAY_SUN,
    ALARM_FIELD_COUNT            // Total fields
} ALARMField;


//function declarations
void CREATE_LOCKSCREEN_WINDOWS();
void handleTimerFieldAdjustment(bool increase);
void updateAppList(char *buf, size_t bufSize, const char **apps, int count);
std::string formatTimerSetter(uint8_t highlightedField, bool confirmMode, uint8_t timerIndex);
void Input_handler_fn_main_screen(uint16_t key);
void WATCH_SCREEN_TRANSITION(WatchMode desiredMode);
void onVertical_input_timer_buff_setter(bool increase, uint8_t fieldIndex, uint8_t timerIndex);
static void on_wm_main_input(uint16_t key);
static void on_wm_stopwatch_input(uint16_t key);
static void on_wm_appmenu_input(uint16_t key);
void render_timer_screen();
void on_wm_timer_input(uint16_t key);
bool SaveTimer();



extern void WatchScreenTask(void* pvParameters);

void CreateWatchscreen();

#endif