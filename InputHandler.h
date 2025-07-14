#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H


#include <Arduino.h>
#include "Wiring.h"
//#pragma once
#include "driver/pulse_cnt.h"
// Constants
#define AllowSimultaneousKeypress true

// Key mappings-map the hardware keys to the ascii versions of keys
#define key_enter 0x23CE  // ⏎
#define key_back  0x232B  // ⌫
#define key_up    0x2191  // ↑
#define key_down  0x2193  // ↓
#define key_left  0x2190  // ←
#define key_right 0x2192  // →

typedef struct encoder_state {
    pcnt_unit_handle_t unit;
    int16_t last_count;
} encoder_state_t;
// Input Device Types
//typedef struct encoder_state encoder_state_t;

#define FixDoubleInputGlitch 1
//encoder makes two pulses for notch

typedef enum{
keyenter,
keyback, 
keyup,
keydown,
keyleft,
keyright
} hardwarekeys;
//yes, it's different than the index of key mappings. this is for state machines that use ONLY the hardware keys, like key combos


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

typedef enum{
EXIT_TO_MAIN,
SLEEP,
FORCE_SHUTDOWN,
REBOOT,
DIAGNOSTICS
}KEYCOMBO;

void handleButton0Interrupt();
void handleButton1Interrupt();

static const uint8_t ENC_DEBOUNCE_MS = 25;  // Increased from 5ms
static const uint8_t ENC_COOLDOWN_MS = 50;   // Minimum time between events

// Encoder state tracking
typedef struct {
    uint8_t last_clk;
    uint8_t last_valid_clk;
    int32_t position;
    bool dir_flag;
    uint32_t last_edge_time;
} EncoderState;

// Only declare the variables (no initialization)


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


void SetupHardwareInput();
void PollEncoders();

void PollEncoderX();
void PollEncoderY();
void PollButtons();
void SetupPCNT();

void RouteInput(S_UserInput uinput, HID_ROUTE_TARGET uout);

#endif // INPUT_HANDLER_H