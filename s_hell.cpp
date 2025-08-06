#include "types.h"
#include "helperfunctions.h"
#include "s_hell.h"
#include "InputHandler.h"
#include "Micro2D_A.h"
#include "globals.h"
#include <cstdint>
#include <Arduino.h>
#include "NFCAPP.h"


//app ui configs
WindowCfg d_ls_c_cfg = { //clock
    14, 64, //xy
    100, 42, //wh
    false, false, //auto align,wraptext
    2, //text size
    true,//borderless?
    tcol_secondary, tcol_background, tcol_primary, // <-- pass addresses!. colors
    1000 //update interval ms
};

 WindowCfg d_ls_b_cfg = {//heart monitor
    86, 0,
    50, 12,
    false, false,
    1,
    true,
    tcol_secondary, tcol_background, tcol_primary,
    1000
};

 WindowCfg d_ls_th_cfg = {//thermometer
    8, 0,
    50, 12,
    false, false,
    1,
    false,
    tcol_secondary, tcol_background, tcol_primary,
    1000
};

AppName CurrentOpenApplicationIndex=APP_LOCK_SCREEN; 
//handles for tasks/apps
//follows finite state machine to ensure correct task starts, management done here.

TaskHandle_t inputTaskHandle; //NEVER changed!!!!

TaskHandle_t currentAppTaskHandle=nullptr; //current app task will be disabled and swapped to the desired app task
TaskHandle_t watchScreenHandle    = nullptr;
TaskHandle_t healthTaskHandle     = nullptr;
TaskHandle_t nfcTaskHandle        = nullptr;
TaskHandle_t settingsTaskHandle   = nullptr;
TaskHandle_t gyroInfoTaskHandle   = nullptr;
TaskHandle_t filesTaskHandle      = nullptr;
TaskHandle_t radioTaskHandle      = nullptr;
TaskHandle_t irRemoteTaskHandle   = nullptr;
TaskHandle_t utilitiesTaskHandle  = nullptr;
TaskHandle_t etoolsTaskHandle     = nullptr;
TaskHandle_t rubberDuckyTaskHandle= nullptr;
TaskHandle_t connectionsTaskHandle= nullptr;
TaskHandle_t smartDevicesTaskHandle=nullptr;
TaskHandle_t diagnosticsTaskHandle=nullptr;
TaskHandle_t gamesTaskHandle      = nullptr;
//functions


TaskHandle_t* GetTaskHandleByIndex(AppName index) {
    switch (index) {
        case APP_LOCK_SCREEN:    return &watchScreenHandle;
        case APP_HEALTH:         return &healthTaskHandle;
        case APP_NFC:            return &nfcTaskHandle;
        case APP_SETTINGS:       return &settingsTaskHandle;
        case APP_GYRO_INFO:      return &gyroInfoTaskHandle;
        case APP_FILES:          return &filesTaskHandle;
        case APP_RADIO:          return &radioTaskHandle;
        case APP_IR_REMOTE:      return &irRemoteTaskHandle;
        case APP_UTILITIES:      return &utilitiesTaskHandle;
        case APP_ETOOLS:         return &etoolsTaskHandle;
        case APP_RUBBERDUCKY:    return &rubberDuckyTaskHandle;
        case APP_CONNECTIONS:    return &connectionsTaskHandle;
        case APP_SMART_DEVICES:  return &smartDevicesTaskHandle;
        case APP_DIAGNOSTICS:    return &diagnosticsTaskHandle;
        case APP_GAMES:          return &gamesTaskHandle;
        default:
            // Fallback to current app handle
            return &currentAppTaskHandle;
    }
}

//taskdefs must be defined
extern void WatchScreenTask(void*); //defined in mainapp
extern void nfcTask(void*);//defined at nfcapp
// Add other task entry points as needed...

bool on_app_change(
    AppName newIndex,
    TaskHandle_t* newTaskHandlePtr,
    AppName oldIndex,
    TaskHandle_t* oldTaskHandlePtr,
    bool deleteOldTask
) {
    // Suspend or delete the old task if valid
    if (oldTaskHandlePtr && *oldTaskHandlePtr) {
        if (oldIndex == APP_LOCK_SCREEN) {
            vTaskSuspend(*oldTaskHandlePtr);
        } else if (deleteOldTask) {
            vTaskDelete(*oldTaskHandlePtr);
            *oldTaskHandlePtr = nullptr;
        } else {
            vTaskSuspend(*oldTaskHandlePtr);
        }
    }

    // Resume or create the new task
    if (newTaskHandlePtr) {
        if (*newTaskHandlePtr) {
            vTaskResume(*newTaskHandlePtr);
        } else {
            // Attempt to auto-create the task
            switch (newIndex) {
                case APP_LOCK_SCREEN:
                    xTaskCreate(WatchScreenTask, "WatchScreen", 2048, nullptr, 1, newTaskHandlePtr);
                    break;
                case APP_NFC:
                    xTaskCreate(nfcTask, "NFC", 4096, nullptr, 1, newTaskHandlePtr);
                    break;
                // Add other app cases and task functions here
                default:
                    Serial.printf("Auto-create failed: Unknown task for app %d\n", newIndex);
                    break;
            }
        }
        currentAppTaskHandle = *newTaskHandlePtr;
    }

    CurrentOpenApplicationIndex = newIndex;
    return true;
}

void transitionApp(AppName newApp, bool deleteOldTask) {
    rst_nav_pos();

    AppName oldApp = CurrentOpenApplicationIndex;
    TaskHandle_t* oldHandle = GetTaskHandleByIndex(oldApp);
    TaskHandle_t* newHandle = GetTaskHandleByIndex(newApp);

    on_app_change(newApp, newHandle, oldApp, oldHandle, deleteOldTask);

    switch (newApp) {
        case APP_NFC:
            NFC_APP_TRANSITION(NAM_READING);
            break;
        // Additional app-specific initializations can go here
        default:
            break;
    }
}
void SleepApp(TaskHandle_t target) {
    if (target != nullptr) {
        // Don't suspend if already suspended
        eTaskState state = eTaskGetState(target);
        if (state != eSuspended) {
            vTaskSuspend(target);
        }
    } else {
        Serial.println("SleepApp: Invalid TaskHandle");
    }
}

void LaunchApp(TaskHandle_t target) {
    if (target != nullptr) {
        // Only resume if the task is suspended
        eTaskState state = eTaskGetState(target);
        if (state == eSuspended) {
            vTaskResume(target);
        }
    } else {
        Serial.println("LaunchApp: Invalid TaskHandle");
    }
}

//scrolling up enters it?
//globalNavPos.x
//i have an addiction for doing ths stupidest shit possible
/*
//so fuck it let's port basic on here
#include <cstring>
#include <cstddef>

// === Program Control ===
enum BASIC_INTERPRETER_CMD_CONTROL {
    EM_BASIC_RUN,      // "RUN"
    EM_BASIC_NEW,      // "NEW"
    EM_BASIC_LOAD,     // "LOAD"
    EM_BASIC_SAVE,     // "SAVE"
    EM_BASIC_VERIFY,   // "VERIFY"
    EM_BASIC_LIST      // "LIST"
};

// === Execution ===
enum BASIC_INTERPRETER_CMD_EXECUTION {
    EM_BASIC_END,      // "END"
    EM_BASIC_STOP,     // "STOP"
    EM_BASIC_CLEAR,    // "CLEAR"
    EM_BASIC_CLS       // "CLS"
}   ;

// === Control Flow ===
enum BASIC_INTERPRETER_CMD_FLOW {
    EM_BASIC_GOTO,     // "GOTO"
    EM_BASIC_IF,       // "IF"
    EM_BASIC_THEN,     // "THEN"
    EM_BASIC_FOR,      // "FOR"
    EM_BASIC_NEXT,     // "NEXT"
    EM_BASIC_STEP,     // "STEP"
    EM_BASIC_GOSUB,    // "GOSUB"
    EM_BASIC_RETURN    // "RETURN"
};

// === I/O ===
enum BASIC_INTERPRETER_CMD_IO {
    EM_BASIC_PRINT,    // "PRINT"
    EM_BASIC_LPRINT,   // "LPRINT"
    EM_BASIC_LLIST,    // "LLIST"
    EM_BASIC_INPUT     // "INPUT"
};

// === Data Management ===
enum BASIC_INTERPRETER_CMD_DATA {
    EM_BASIC_DATA,     // "DATA"
    EM_BASIC_READ,     // "READ"
    EM_BASIC_RESTORE,  // "RESTORE"
    EM_BASIC_DIM       // "DIM"
};

// === Miscellaneous ===
enum EM_BASIC_INTERPRETER_CMD_MISC {
    EM_BASIC_REM,      // "REM"
    EM_BASIC_POKE,     // "POKE"
    EM_BASIC_SOUND     // "SOUND"
};
// === UNIFIED ENUM (OPTIONAL) ===
// If you want one type for all commands:
enum BASIC_INTERPRETER_CMD {
    CMD_RUN, CMD_NEW, CMD_LOAD, CMD_SAVE, CMD_VERIFY, CMD_LIST,
    CMD_END, CMD_STOP, CMD_CLEAR, CMD_CLS,
    CMD_GOTO, CMD_IF, CMD_THEN, CMD_FOR, CMD_NEXT, CMD_STEP, CMD_GOSUB, CMD_RETURN,
    CMD_PRINT, CMD_LPRINT, CMD_LLIST, CMD_INPUT,
    CMD_DATA, CMD_READ, CMD_RESTORE, CMD_DIM,
    CMD_REM, CMD_POKE, CMD_SOUND,
    CMD_UNKNOWN
};

// === STRUCT ===

struct BasicKeyword {
    const char* keyword;
    BASIC_INTERPRETER_CMD command;
};

// === MASTER TABLE ===
//yes the cmd thing doesn't match shut the fuck up i'll fix it later


constexpr BasicKeyword BASIC_KEYWORDS[] = {
    {"RUN", CMD_RUN},
    {"NEW", CMD_NEW},
    {"LOAD", CMD_LOAD},
    {"SAVE", CMD_SAVE},
    {"VERIFY", CMD_VERIFY},
    {"LIST", CMD_LIST},

    {"END", CMD_END},
    {"STOP", CMD_STOP},
    {"CLEAR", CMD_CLEAR},
    {"CLS", CMD_CLS},

    {"GOTO", CMD_GOTO},
    {"IF", CMD_IF},
    {"THEN", CMD_THEN},
    {"FOR", CMD_FOR},
    {"NEXT", CMD_NEXT},
    {"STEP", CMD_STEP},
    {"GOSUB", CMD_GOSUB},
    {"RETURN", CMD_RETURN},

    {"PRINT", CMD_PRINT},
    {"LPRINT", CMD_LPRINT},
    {"LLIST", CMD_LLIST},
    {"INPUT", CMD_INPUT},

    {"DATA", CMD_DATA},
    {"READ", CMD_READ},
    {"RESTORE", CMD_RESTORE},
    {"DIM", CMD_DIM},

    {"REM", CMD_REM},
    {"POKE", CMD_POKE},
    {"SOUND", CMD_SOUND}
};

// === MATCHER FUNCTION ===

constexpr size_t BASIC_KEYWORDS_COUNT = sizeof(BASIC_KEYWORDS) / sizeof(BASIC_KEYWORDS[0]);

BASIC_INTERPRETER_CMD matchBasicKeyword(const char* token) {
    for (size_t i = 0; i < BASIC_KEYWORDS_COUNT; ++i) {
        if (strcmp(token, BASIC_KEYWORDS[i].keyword) == 0) {
            return BASIC_KEYWORDS[i].command;
        }
    }
    return CMD_UNKNOWN; // fallback for not found
}*/
