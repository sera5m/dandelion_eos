#include "types.h"
#include "Wiring.h"
#include "MainApp.h"
#include "mdl_clock.h"
#include "helperfunctions.h"
#include <sstream>
#include <cstring>
#include <cstddef>
#include <stdbool.h>
#include <ctime>
#include <vector>
#include <cstdint>
#include <stdio.h>
//give up and shotgun them all in because fuck me
#include "InputHandler.h"

void handleTimerFieldAdjustment(bool increase) {
    // Safety check - ensure we're editing a valid timer
    if (globalNavPos.y >= NUM_TIMERS) return;
    
    // Get reference to the timer we're editing
    usr_alarm_st& currentAlarm = usrmade_timers[globalNavPos.y];
    
    const int8_t direction = increase ? 1 : -1;
    uint8_t days_bitmask;
    uint8_t color[3];
    
    split_u32_to_24(currentAlarm.LightColor, &days_bitmask, color);

    switch (currentTimerField) {
        case TIMER_FIELD_HOUR_TENS: {
            int tens = currentAlarm.hours / 10;
            tens = (tens + direction + 3) % 3; // Wrap 0-2
            int ones = min(currentAlarm.hours % 10, tens == 2 ? 3 : 9);
            currentAlarm.hours = tens * 10 + ones;
            break;
        }
        
        case TIMER_FIELD_HOUR_ONES: {
            int tens = currentAlarm.hours / 10;
            int max_ones = (tens == 2) ? 3 : 9;
            int ones = (currentAlarm.hours % 10 + direction + max_ones + 1) % (max_ones + 1);
            currentAlarm.hours = tens * 10 + ones;
            break;
        }
        
        // todo check if i'm issing anything here
        
        case TIMER_FIELD_ALARM_ACTION: {
            int action = (static_cast<int>(currentAlarm.E_AlarmAction) + direction);
            action = (action + ALARM_ACTION_MAX) % ALARM_ACTION_MAX;
            currentAlarm.E_AlarmAction = static_cast<alarmAction>(action);
            break;
        }
        
        // Recombine LightColor
        currentAlarm.LightColor = ((uint32_t)days_bitmask << 24) | 
                                (color[0] << 16) | 
                                (color[1] << 8) | 
                                color[2];
    }
}

void updateAppList(char *buf, size_t bufSize, const char **apps, int count) { //part of appmenu
    buf[0] = '\0';
    for (int i = 0; i < min(count, MAX_VISIBLE); ++i) {
        if (i == globalNavPos.y) {  // Now checking y position
            strncat(buf, "<setcolor(0xdbbf)>[", bufSize);
            strncat(buf, apps[i], bufSize);
            strncat(buf, "]<setcolor(0x07ff)>", bufSize);
        } else {
            strncat(buf, apps[i], bufSize);
        }
        strncat(buf, "<n>", bufSize);
    }//end for, also i should not use for lol


// Pad with blank lines if needed
int visibleLines = APP_COUNT < MAX_VISIBLE ? APP_COUNT : MAX_VISIBLE;
 for (int i = visibleLines; i < MAX_VISIBLE; ++i) {
  strncat(buf, "<n>", bufSize - strlen(buf) - 1);
}

    // Optionally ensure trailing newline if needed
    //strncat(buf, "<n>", bufSize - strlen(buf) - 1);
//end fn update applist

}//end fn


std::string formatTimerSetter(uint8_t highlightedField, bool confirmMode, uint8_t timerIndex) {
    // Validate timer index first
    if (timerIndex >= NUM_TIMERS) {
        return "Invalid timer";
    }

    // Get reference to the current timer
    usr_alarm_st& currentTimer = usrmade_timers[timerIndex];
    uint8_t days_bitmask;
    uint8_t color[3];
    split_u32_to_24(currentTimer.LightColor, &days_bitmask, color);

    std::ostringstream oss;
    
    // 1. Format Time Section
    oss << "Create new timer: <n>Duration: ";
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", currentTimer.hours, currentTimer.minutes);
    
    // Highlight individual time components
    for (uint8_t i = 0; i < 5; i++) {
        if (i == 2) { // Colon separator
            oss << timeStr[i];
            continue;
        }
        
        bool shouldHighlight = false;
        switch (i) {
            case 0: shouldHighlight = (highlightedField == TIMER_FIELD_HOUR_TENS); break;
            case 1: shouldHighlight = (highlightedField == TIMER_FIELD_HOUR_ONES); break;
            case 3: shouldHighlight = (highlightedField == TIMER_FIELD_MIN_TENS); break;
            case 4: shouldHighlight = (highlightedField == TIMER_FIELD_MIN_ONES); break;
        }
        
        if (shouldHighlight) {
            oss << "<setcolor(0xF005)>[" << timeStr[i] << "]<setcolor(0x07ff)>";
        } else {
            oss << timeStr[i];
        }
    }

    // 2. Format Alarm Action
    oss << "<n><n>Alert: ";
    if (highlightedField == TIMER_FIELD_ALARM_ACTION) {
        oss << "<setcolor(0xF005)>[" << AlarmActionNames[currentTimer.E_AlarmAction] << "]<setcolor(0x07ff)>";
    } else {
        oss << AlarmActionNames[currentTimer.E_AlarmAction];
    }

    // 3. Format Snooze Duration
    oss << "<n><n>Snooze: ";
    if (highlightedField == TIMER_FIELD_SNOOZE) {
        oss << "<setcolor(0xF005)>[" << static_cast<int>(currentTimer.SnoozeDur) << "min]<setcolor(0x07ff)>";
    } else {
        oss << static_cast<int>(currentTimer.SnoozeDur) << "min";
    }

    // 4. Format Action Buttons
    oss << "<n><n>enter/back";
    if (confirmMode) {
        oss << "<setcolor(0xF005)>[SAVE?]";
    } else {
        oss << "<setcolor(0xF005)>[edit]";
    }

    return oss.str();
}
