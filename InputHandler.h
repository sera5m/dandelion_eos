#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "Wiring.h"

// Constants
#define AllowSimultaneousKeypress true

// Key mappings
#define key_enter 0x23CE
#define key_back 0x232B
#define key_up 0x2191
#define key_down 0x2193
#define key_left 0x2190
#define key_right 0x2192

// Input Device Types
typedef enum {
mouse,
keyboard,
device
} inputDeviceType;

// HID Routing Targets
typedef enum {
R_wakey,
R_toProc,
R_os
} HID_ROUTE_TARGET;

// User Input Structure
typedef struct {
uint16_t key;
bool isDown;
} S_UserInput;

// External Variables
extern volatile bool button0Pressed;
extern volatile bool button1Pressed;
extern volatile int encoder0_Pos;
extern volatile int encoder1_Pos;
extern HID_ROUTE_TARGET currentinputTarget;

// Function Declarations
void IRAM_ATTR handleButton0Interrupt();
void IRAM_ATTR handleButton1Interrupt();
void SetupHardwareInput();
void PollEncoders();
void RouteInput(S_UserInput uinput, HID_ROUTE_TARGET uout);

#endif // INPUT_HANDLER_H