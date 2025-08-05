#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>

// developer configs
#define WATCHSCREEN_BUF_SIZE 512

// userspace configs-config them in globals.cpp

extern uint16_t tcol_primary;
extern uint16_t tcol_secondary;
extern uint16_t tcol_tertiary;
extern uint16_t tcol_highlight;
extern uint16_t tcol_background;

// var references to set for the theme
// 0x07FF, // teal
// 0x77F9, // i can't find a good green
// 0xE4FF, // lavender
// 0xD7FD, // very light green highlight
// 0x29E6  // background

//#define evilcodebase 0 // Alternatively, define this externally


//enable for EVIL!!!
#ifndef evilcodebase
#define evilcodebase 0
#endif

#if evilcodebase
  #undef true
  #define true false
  #undef >
  #define > <
#endif

#endif // GLOBALS_H
