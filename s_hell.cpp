#include "types.h"
#include "helperfunctions.h"
#include "s_hell.h"
#include "InputHandler.h"

void transitionApp(uint8_t index) {
    AppName app = (AppName)index;
rst_nav_pos(); //reset mouse pos between apps

    switch (app) {
        //set 
        

        case APP_LOCK_SCREEN:
            // Do something for lock screen
            CurrentOpenApplicationIndex=APP_LOCK_SCREEN; //note! need to set this on sucess, move this later to outside just this mode. this is the current open app n should only be set on success!
            break;
        case APP_HEALTH:
            // Do something for health app
            break;
        case APP_NFC:
            // ...
            break;
        case APP_SETTINGS:
            // ...
            break;
        case APP_GYRO_INFO:
            // ...
            break;
        case APP_FILES:
            // ...
            break;
        case APP_RADIO:
            // ...
            break;
        case APP_IR_REMOTE:
            // ...
            break;
        case APP_UTILITIES:
            // ...
            break;
        case APP_ETOOLS:
            // ...
            break;
        case APP_RUBBERDUCKY:
            // ...
            break;
        case APP_CONNECTIONS:
            // ...
            break;
        case APP_SMART_DEVICES:
            // ...
            break;
        case APP_DIAGNOSTICS:
            // ...
            break;

        case APP_COUNT:  // Usually no action here, just for bounds
        default:
            // Handle invalid selection gracefully
            break;
    }
    //run a verif step in the future, todo. LIKELY a wait too, so it can even set the app
    CurrentOpenApplicationIndex=app; //set via lazymaxxing
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
};

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