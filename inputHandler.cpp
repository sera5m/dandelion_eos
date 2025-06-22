#include "Wiring.h"
#include "esp_sleep.h"
#include <Arduino.h>
#include "kernel.ino"

#define AllowSimultaneousKeypress true

bool enc0_dir_right = false;
bool enc0_dir_left  = false;
bool enc1_dir_up    = false;
bool enc1_dir_down  = false;
// Remember pin definitions are in Wiring.h

volatile bool button0Pressed = false;
volatile bool button1Pressed = false;
  volatile bool prevButton0State=0;
  volatile bool prevButton1State=0;


volatile int encoder0_Pos = 0;
volatile int encoder1_Pos = 0;

int lastCLK_0 = 1;
int lastCLK_1 = 1;


//in the .h folder


HID_ROUTE_TARGET currentinputTarget = R_os; // Default route to OS

void IRAM_ATTR handleButton0Interrupt() {
  //button0Pressed=true; //should probably just uhhh have it say the button isnt pressed.... sometime whenever it's up. can't put anythin heavy in here so whatever man
}

void IRAM_ATTR handleButton1Interrupt() {
 //button1Pressed = true;
}

//referencing the wiring.
/*//x axis,sel
#define ENCODER0_CLK_PIN 42
#define ENCODER0_DT_PIN  14
#define ENCODER0_SW_PIN  7

//y axis, back

#define ENCODER1_CLK_PIN 21
#define ENCODER1_DT_PIN  19
#define ENCODER1_SW_PIN  18
*/


void SetupHardwareInput() {
  pinMode(ENCODER0_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER0_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER0_SW_PIN, INPUT_PULLUP);

  pinMode(ENCODER1_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER1_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER1_SW_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER0_SW_PIN), handleButton0Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER1_SW_PIN), handleButton1Interrupt, FALLING);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)ENCODER1_SW_PIN, 0);  // 0 = LOW triggers wake
  esp_sleep_enable_ext0_wakeup((gpio_num_t)ENCODER0_SW_PIN, 0); 

  esp_sleep_enable_ext1_wakeup(
      (1ULL << ENCODER0_SW_PIN) |
      (1ULL << ENCODER1_SW_PIN),
      ESP_EXT1_WAKEUP_ANY_LOW //one of these is the enter or back button, can't remember which
  );
}


extern QueueHandle_t lockscreenQueue;
extern QueueHandle_t processInputQueue;

void RouteInput(S_UserInput uinput, HID_ROUTE_TARGET uout) {
    switch (uout) {
        case R_wakey:
            Serial.println("Waking up device...");
            break;
        case R_toProc:
            if (processInputQueue) {
                if (xQueueSend(processInputQueue, &uinput, 0) != pdPASS)
                    Serial.println("Process queue full.");
            }
            break;
        case R_os:
            if (lockscreenQueue) {
                if (xQueueSend(lockscreenQueue, &uinput, 0) != pdPASS)
                    Serial.println("OS queue full.");
            }
            break;
        default:
            break;
    }
}


//remember!
//#define key_enter 0x23CE  // ⏎
//#define key_back  0x232B  // ⌫
//#define key_up    0x2191  // ↑
//#define key_down  0x2193  // ↓
//#define key_left  0x2190  // ←
//#define key_right 0x2192  // →
/*
//x axis,sel
#define ENCODER0_CLK_PIN 42
#define ENCODER0_DT_PIN  14
#define ENCODER0_SW_PIN  7

//y axis, back

#define ENCODER1_CLK_PIN 21
#define ENCODER1_DT_PIN  19
#define ENCODER1_SW_PIN  18
*/


// Fixed PollEncoders function for robust encoder and button handling
// Improvements:
// 1. Added per-button debounce timing
// 2. Removed unconditional key_enter release that caused stuck state
// 3. Unified edge detection: only send press/release on actual transitions
// 4. Simplified encoder "no movement" logic
// 5. Ensured auto-clear of transient flags outside input dispatch

//--6. fixed two pins and modified .h file


// Constants for debounce intervals (ms)
static const uint8_t ENC_DEBOUNCE_MS = 2;
static const uint8_t  BTN_DEBOUNCE_MS = 50;
// Encoder handling split from PollEncoders
// Constants for debounce intervals (ms)



void PollEncoderX() {
    int clk0 = digitalRead(ENCODER0_CLK_PIN);
    if (clk0 != lastCLK_0) {
        if (clk0 == LOW) {
            bool cw = (digitalRead(ENCODER0_DT_PIN) != clk0);
            encoder0_Pos += (cw ? 1 : -1);

            if (cw && !enc0_dir_right) {
                RouteInput({key_right, true}, currentinputTarget);
                enc0_dir_right = true;
            } else if (!cw && !enc0_dir_left) {
                RouteInput({key_left, true}, currentinputTarget);
                enc0_dir_left = true;
            }
        }
        lastCLK_0 = clk0;
    } else if (clk0 == HIGH && lastCLK_0 != clk0) {
        if (enc0_dir_right) {
            RouteInput({key_right, false}, currentinputTarget);
            enc0_dir_right = false;
        }
        if (enc0_dir_left) {
            RouteInput({key_left, false}, currentinputTarget);
            enc0_dir_left = false;
        }
        lastCLK_0 = clk0;
    }
}

void PollEncoderY() {
    int clk1 = digitalRead(ENCODER1_CLK_PIN);
    if (clk1 != lastCLK_1) {
        if (clk1 == LOW) {
            bool cw = (digitalRead(ENCODER1_DT_PIN) != clk1);
            encoder1_Pos += (cw ? 1 : -1);

            if (cw && !enc1_dir_down) {
                RouteInput({key_down, true}, currentinputTarget);
                enc1_dir_down = true;
            } else if (!cw && !enc1_dir_up) {
                RouteInput({key_up, true}, currentinputTarget);
                enc1_dir_up = true;
            }
        }
        lastCLK_1 = clk1;
    } else if (clk1 == HIGH && lastCLK_1 != clk1) {
        if (enc1_dir_down) {
            RouteInput({key_down, false}, currentinputTarget);
            enc1_dir_down = false;
        }
        if (enc1_dir_up) {
            RouteInput({key_up, false}, currentinputTarget);
            enc1_dir_up = false;
        }
        lastCLK_1 = clk1;
    }
}

// --- Button logic split out ---
 unsigned long lastBtn0Time = 0;
unsigned long lastBtn1Time = 0;
 unsigned long lastEncCheck = 0;
void PollButtons() {
    unsigned long now = millis();

    bool rawBtn0 = digitalRead(ENCODER0_SW_PIN) == LOW; // assuming active-low
    bool rawBtn1 = digitalRead(ENCODER1_SW_PIN) == LOW;

    if (rawBtn0 != prevButton0State && now - lastBtn0Time > BTN_DEBOUNCE_MS) {
        lastBtn0Time = now;
        prevButton0State = rawBtn0;
        RouteInput({key_enter, rawBtn0}, currentinputTarget);
    }

    if (rawBtn1 != prevButton1State && now - lastBtn1Time > BTN_DEBOUNCE_MS) {
        lastBtn1Time = now;
        prevButton1State = rawBtn1;
        RouteInput({key_back, rawBtn1}, currentinputTarget);
    }
}



void PollEncoders() {
    
    unsigned long now = millis();
    if (now - lastEncCheck < ENC_DEBOUNCE_MS) return;
    lastEncCheck = now;

    PollEncoderX();
    PollEncoderY();
}


/*
//unsure how i'll handle delays for this, i'll figure it out later
uint_16t HARDWAREKEYINPUTBITMASK=0x0000;

void keycombo_solver(uint_16 bitmask){ //function takes bitmask and returns the combo item (enum KEYCOMBO)
  //last 4 keys are stored in a uint_16. the range of keys we have is 0-5 (6 keys) fitting in 0xabcd instead of an array for less overhead. we poll this code really fast so that's why i'm doing bit twiddling
bitmask



}*/

