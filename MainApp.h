#ifndef MAINAPP_H
#define MAINAPP_H
#include <cstdint>
#include "types.h"
#include "Wiring.h"
#include "mdl_clock.h"
#include "helperfunctions.h"
#include "InputHandler.h"
#include "mdl_clock.h"
#pragma once

#define MAX_VISIBLE 15
char buf_applist[25*MAX_VISIBLE]; //

 EditState timerEditState; //in types.h, options off,running,confirm
 uint8_t currentTimerField; 

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
void handleTimerFieldAdjustment(bool increase);
void updateAppList(char *buf, size_t bufSize, const char **apps, int count);
std::string formatTimerSetter(uint8_t highlightedField, bool confirmMode, uint8_t timerIndex);


#endif