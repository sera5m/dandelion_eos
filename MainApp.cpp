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
#include "s_hell.h"
#include "Micro2D_A.h"
#include "globals.h"
#include <Arduino.h>

//extern std::unique_ptr<WindowManager> WinManagerInstance;

 std::shared_ptr<Window> Win_GeneralPurpose; 
 std::shared_ptr<Window> lockscreen_biomon;
 std::shared_ptr<Window> lockscreen_thermometer;

 void CREATE_LOCKSCREEN_WINDOWS(){
    Win_GeneralPurpose = std::make_shared<Window>("Win_GeneralPurpose", d_ls_c_cfg, "HH:MM:SS");
WindowManager::getInstance().registerWindow(Win_GeneralPurpose);
//do not use this here!//DBG_PRINTLN("Clock OK");

        lockscreen_biomon = std::make_shared<Window>("lockscreen_biomon", d_ls_b_cfg, "XXXbpm");
WindowManager::getInstance().registerWindow(lockscreen_biomon);
//do not use this here!//DBG_PRINTLN("Biomon OK");

    lockscreen_thermometer = std::make_shared<Window>("lockscreen_thermometer", d_ls_th_cfg, "XXXC");
WindowManager::getInstance().registerWindow(lockscreen_thermometer);

}
int WatchScreenUpdateInterval=500;
char watchscreen_buf[WATCHSCREEN_BUF_SIZE];
char buf_applist[25*MAX_VISIBLE]; 
// Timer editing state and field
EditState timerEditState;
uint8_t currentTimerField;

// Watch mode and timers
WatchMode currentWatchMode = WM_MAIN;
int stopwatchElapsed = 0;
uint8_t selectedTimerIndex = 0;
uint8_t watchModeIndex=0;

// UI state
bool is_watch_screen_in_menu = false;
bool isConfirming = false;

// Stopwatch
bool stopwatchRunning = false;
unsigned long stopwatchStart = 0;

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



void Input_handler_fn_main_screen(uint16_t key) {
    // Handle global navigation keys
        Serial.printf("main_screen_input");
    Serial.println(key);
    
    // Special case: Block global nav keys when timer is editing
    if (currentWatchMode == WM_TIMER && timerEditState != EDIT_OFF) {
        if (key == key_left || key == key_right) {
            on_wm_timer_input(key); // Let timer handle these
            return;
        }
    }

   switch(key) {
        case key_left:
            watchModeIndex = (watchModeIndex == 0) ? WM_COUNT - 1 : watchModeIndex - 1;
            currentWatchMode = (WatchMode)watchModeIndex;
            WATCH_SCREEN_TRANSITION(currentWatchMode);
            return;
            
        case key_right:
            watchModeIndex = (watchModeIndex + 1) % WM_COUNT;
            currentWatchMode = (WatchMode)watchModeIndex;
            WATCH_SCREEN_TRANSITION(currentWatchMode);
            return;

        //todo have popup menu or whatever here, with actual window show 

  case key_up:
   // strip.setPixelColor(0, strip.Color(255, 255, 255)); strip.show();    Serial.println("Flashlight ON");
    break;
    
case key_down:
    //strip.setPixelColor(0, strip.Color(0, 0, 0));     strip.show();    Serial.println("Flashlight OFF");
    break;

         default:
         break;   
   }//hald the thing
    // Route to mode-specific handler
    switch(currentWatchMode) {
        case WM_MAIN:        on_wm_main_input(key); break;
        case WM_STOPWATCH:   on_wm_stopwatch_input(key); break;
        case WM_APPMENU:     on_wm_appmenu_input(key); break;
        case WM_TIMER:       on_wm_timer_input(key); break;
        // Add stubs for other modes
        case WM_ALARMS:      /* Implement later */ break;
        case WM_NTP_SYNCH:   /* Implement later */ break;
        case WM_SET_TIME:    /* Implement later */ break;
        case WM_SET_TIMEZONE:/* Implement later */ break;
        default: break;
    }
}


void on_wm_timer_input(uint16_t key) {
    Serial.printf("Timer input: 0x%04X State: %d\n", key, timerEditState);
    
    switch(timerEditState) {
        case EDIT_OFF: {
            // List navigation mode
            Navlimits_.x = 0;
            Navlimits_.y = NUM_TIMERS-1;
            Navlimits_.z = 0;
            
            switch(key) {
                case key_enter:
                    if (NUM_TIMERS > 0) {  // Only enter edit mode if timers exist
                        timerEditState = EDIT_RUNNING;
                        globalNavPos.x = TIMER_FIELD_HOUR_TENS; // Start with hour tens
                        selectedTimerIndex = globalNavPos.y;     // Set current edit timer
                    }
                    break;
                    
                case key_back:
                    currentWatchMode = WM_MAIN;
                    WATCH_SCREEN_TRANSITION(WM_MAIN);
                    return;
                    
                case key_up:
                case key_down:
                    changeNavPos(int16vect{0, static_cast<int16_t>((key == key_up) ? -1 : 1), 0}, 
                               true, Navlimits_);
                    break;
                    
                default: 
                    break;
            }
            break;
        }

        case EDIT_RUNNING: {
            // Field editing mode
            if (selectedTimerIndex >= NUM_TIMERS) {  // Safety check
                timerEditState = EDIT_OFF;
                break;
            }

            Navlimits_.x = TIMER_FIELD_COUNT-1;
            Navlimits_.y = 0;
            Navlimits_.z = 0;
            
            switch(key) {
                case key_enter:
                    timerEditState = EDIT_CONFIRM;
                    globalNavPos.x = 0; // Select "Save" option by default
                    break;
                    
                case key_back:
                    timerEditState = EDIT_OFF;
                    break;
                    
                case key_up:
                    handleTimerFieldAdjustment(true); // Increase current field
                    break;
                    
                case key_down:
                    handleTimerFieldAdjustment(false); // Decrease current field
                    break;
                    
                case key_left:
                case key_right:
                    changeNavPos(int16vect{static_cast<int16_t>((key == key_left) ? -1 : 1), 0, 0}, 
                               true, Navlimits_);
                    currentTimerField = static_cast<TimerField>(globalNavPos.x);
                    break;
                    
                default: 
                    break;
            }
            break;
        }

        case EDIT_CONFIRM: {
            // Save confirmation mode
            Navlimits_.x = 1;  // Toggle between Save (0) and Cancel (1)
            Navlimits_.y = 0;
            Navlimits_.z = 0;
            
            switch(key) {
                case key_enter:
                    if (globalNavPos.x == 0) {  // Save selected
                        // Data is already in usrmade_timers, just persist it
                        if (SaveTimer()) {  // Implement this function
                            Serial.println("Alarms saved successfully");
                        } else {
                            Serial.println("Error saving alarms");
                        }
                    }
                    timerEditState = EDIT_OFF;
                    break;
                    
                case key_back:
                    timerEditState = EDIT_RUNNING;
                    break;
                    
                case key_left:
                case key_right:
                    changeNavPos(int16vect{static_cast<int16_t>((key == key_left) ? -1 : 1), 0, 0}, 
                               false, Navlimits_); // No wrap for confirmation
                    break;
                    
                default: 
                    break;
            }
            break;
        }
    }
    
    render_timer_screen();
}


void render_timer_screen() {
    watchscreen_buf[0] = '\0'; // Always start with empty buffer
    
    if (timerEditState == EDIT_OFF) {
        // List view mode
        snprintf(watchscreen_buf, WATCHSCREEN_BUF_SIZE,
               "<setcolor(0xF005)>Timer %d/%d<setcolor(0x07ff)><n><n>", selectedTimerIndex + 1, NUM_TIMERS);
        
        for (int i = 0; i < min(NUM_TIMERS, 5); i++) { // Limit display to 5
            char alarmBuf[30];
            snprintf(alarmBuf, sizeof(alarmBuf), "%s%02d:%02d %s<n>",
                   (i == selectedTimerIndex) ? "> " : "  ",
                   usrmade_timers[i].hours,
                   usrmade_timers[i].minutes,
                   AlarmActionNames[usrmade_timers[i].E_AlarmAction]);
            
            strncat(watchscreen_buf, alarmBuf, 
                   WATCHSCREEN_BUF_SIZE - strlen(watchscreen_buf) - 1);
        }
        
        // Only show instruction if there are timers
        if (NUM_TIMERS > 0) {
            strncat(watchscreen_buf, "<n>^/v Select  <- Edit",
                   WATCHSCREEN_BUF_SIZE - strlen(watchscreen_buf) - 1);
        }
    }
    else {
        // Edit mode
            std::string alarmStr = formatTimerSetter(currentTimerField, 
                                          timerEditState == EDIT_CONFIRM,
                                          globalNavPos.y);
        strncpy(watchscreen_buf, alarmStr.c_str(), WATCHSCREEN_BUF_SIZE-1);
    }
    
    watchscreen_buf[WATCHSCREEN_BUF_SIZE-1] = '\0';
    Win_GeneralPurpose->updateContent(watchscreen_buf);
}

void onVertical_input_timer_buff_setter(bool increase, uint8_t fieldIndex, uint8_t timerIndex) {
    if (timerIndex >= NUM_TIMERS) return;
    
    usr_alarm_st& alarm = usrmade_timers[timerIndex];
    const int8_t direction = increase ? 1 : -1;

    switch(fieldIndex) {
        case TIMER_FIELD_HOUR_TENS:
            alarm.hours = (alarm.hours + (increase ? 10 : -10)) % 24;
            if (alarm.hours < 0) alarm.hours += 24;
            break;
            
        case TIMER_FIELD_HOUR_ONES:
            alarm.hours = (alarm.hours + (increase ? 1 : -1)) % 24;
            if (alarm.hours < 0) alarm.hours += 24;
            break;
            
        case TIMER_FIELD_MIN_TENS:
            alarm.minutes = (alarm.minutes + (increase ? 10 : -10)) % 60;
            if (alarm.minutes < 0) alarm.minutes += 60;
            break;
            
        case TIMER_FIELD_MIN_ONES:
            alarm.minutes = (alarm.minutes + (increase ? 1 : -1)) % 60;
            if (alarm.minutes < 0) alarm.minutes += 60;
            break;
            
        case TIMER_FIELD_ALARM_ACTION: {
            int action = static_cast<int>(alarm.E_AlarmAction) + direction;
            if (action < 0) action = ALARM_ACTION_MAX - 1;
            if (action >= ALARM_ACTION_MAX) action = 0;
            alarm.E_AlarmAction = static_cast<alarmAction>(action);
            break;
        }
            
        case TIMER_FIELD_SNOOZE:
            alarm.SnoozeDur = constrain(alarm.SnoozeDur + (increase ? 1 : -1), 1, 30);
            break;
    }
}

// Mode-specific input handlers
static void on_wm_main_input(uint16_t key) {
    // Currently no special handling for MAIN mode
}

static void on_wm_stopwatch_input(uint16_t key) {
    switch(key) {
        case key_enter:
            if (stopwatchRunning) {
                stopwatchElapsed += millis() - stopwatchStart;
                stopwatchRunning = false;
            } else {
                stopwatchStart = millis();
                stopwatchRunning = true;
            }
            break;
        default:
            break;
    }
}

static void on_wm_appmenu_input(uint16_t key) {
    switch(key) {
        case key_enter:
            transitionApp(globalNavPos.y);
            break;
            
        case key_up:
        case key_down:
            changeNavPos(int16vect{0, (key == key_up) ? -1 : 1, 0}, 
                       true, 
                       Navlimits_);
            updateAppList(buf_applist, sizeof(buf_applist), appNames, APP_COUNT);
            Win_GeneralPurpose->updateContent(buf_applist);
            break;
            
        case key_back:
            currentWatchMode = WM_MAIN;
            is_watch_screen_in_menu = false;
            WATCH_SCREEN_TRANSITION(WM_MAIN);
            globalNavPos = {0, 0, 0};
            break;
            
        default:
            break;
    }
}
// timer_input.c
// watch_screen.c


void WATCH_SCREEN_TRANSITION(WatchMode desiredMode){
   rst_nav_pos();//reset mouse
  Win_GeneralPurpose->updateContent("");//remove content, avoid visual only bug

switch (desiredMode){
  
//tft.fillScreen(tcol_background);//clean everything

                case WM_MAIN:
            
            Navlimits_ = {0, 0, 0};
                WatchScreenUpdateInterval=500;
                //update bg? 
                //Win_GeneralPurpose->updateContent("");//fixes bug with text overflow
                Win_GeneralPurpose->setWinTextSize(2);
                Win_GeneralPurpose->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true); //reset to original config size reguardless of original config
                
                break;
                
                case WM_STOPWATCH:
                    Navlimits_ = {0, 1, 0}; //there's nothing to nav here, just enter and such
                    WatchScreenUpdateInterval=120;//update more frequently. unfortunately, there's still an issue with latency so we'll keep 
                    //Win_GeneralPurpose->updateContent("");//fixes bug with text overflow
                    Win_GeneralPurpose->setWinTextSize(2);
                    Win_GeneralPurpose->ResizeWindow(128, d_ls_c_cfg.height,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x-14, d_ls_c_cfg.y,true);//move 14 pixels to the left to offset more digits.
                                                        //x now has effective increased size of 28px. probably better if i did this as a new struct but who cares. 
                         //resize window and force update as of 6/25/25 have the ability to not force the screen to update, preventing graphical glitches
                 
                 break;
                

                case WM_ALARMS:
                Navlimits_ = {0, 1, 0};//expand nav limits later navlimits
                    // TODO: Display upcoming alarms or alarm setup screen
                    WatchScreenUpdateInterval=350;
                    Win_GeneralPurpose->setWinTextSize(1);
                    Win_GeneralPurpose->ResizeWindow(128, 112,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(0,16,false); //xy

                 break;

                case WM_TIMER:
                Navlimits_ = {0, NUM_TIMERS-1, 0};//this needs to change inside the timer itself
                WatchScreenUpdateInterval=350;
                Win_GeneralPurpose->setWinTextSize(1);
               Win_GeneralPurpose->ResizeWindow(128, 112,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(0,16,false);
                  break;

                case WM_NTP_SYNCH:
                WatchScreenUpdateInterval=500;
                Win_GeneralPurpose->setWinTextSize(2);
                Win_GeneralPurpose->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,false);
                 Win_GeneralPurpose->updateContent("time synch, wifi");

                  break;

                case WM_SET_TIME:
                WatchScreenUpdateInterval=500;
                Win_GeneralPurpose->setWinTextSize(2);
                Win_GeneralPurpose->ResizeWindow(d_ls_c_cfg.width, d_ls_c_cfg.height,false);
                Win_GeneralPurpose->MoveWindow(d_ls_c_cfg.x, d_ls_c_cfg.y,true);

                    break;

                case WM_SET_TIMEZONE:
                WatchScreenUpdateInterval=350;
                    Win_GeneralPurpose->setWinTextSize(1);
                    Win_GeneralPurpose->ResizeWindow(112, 112,false);//expand for more digits, .xyz expand by  pixels
                    Win_GeneralPurpose->MoveWindow(16,16,false);

                    break;

                case WM_APPMENU:
                Navlimits_ = {0, 1, 0};
                WatchScreenUpdateInterval=500;
                     Navlimits_ = {0, APP_COUNT-1, 0};
                    Win_GeneralPurpose->setWinTextSize(1); //reduce text size to handle list on screen correctly
                    Win_GeneralPurpose->ResizeWindow(128, 128,false);
                    Win_GeneralPurpose->MoveWindow(0,0,true);
                   
                break;    

                default:
                    Serial.println("Unknown WatchMode!");
                    WatchScreenUpdateInterval=600;
                    Win_GeneralPurpose->updateContent("ERROR: Bad Mode");
                   
                break;
            }//end switch
            tft_Fillscreen(tcol_background); //clean the scren up, prep 4 next udpate
}//end fn       

// Helper function to persist alarms to storage
bool SaveTimer() {
    // Implementation depends on your storage system
    // Example for EEPROM:
    /*
    EEPROM.put(ALARMS_STORAGE_ADDR, usrmade_timers);
    return EEPROM.commit();
    */
    return true; // Stub implementation
}