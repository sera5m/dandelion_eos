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

volatile int encoder0_Pos = 0;
volatile int encoder1_Pos = 0;

int lastCLK_0 = 1;
int lastCLK_1 = 1;


//in the .h folder


HID_ROUTE_TARGET currentinputTarget = R_os; // Default route to OS

void IRAM_ATTR handleButton0Interrupt() {
  button0Pressed=true; //should probably just uhhh have it say the button isnt pressed.... sometime whenever it's up. can't put anythin heavy in here so whatever man
}

void IRAM_ATTR handleButton1Interrupt() {
 button1Pressed = true;
}




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
      ESP_EXT1_WAKEUP_ANY_LOW
  );
}


extern QueueHandle_t lockscreenQueue;
extern QueueHandle_t processInputQueue;

void RouteInput(S_UserInput uinput, HID_ROUTE_TARGET uout) {
    switch (uout) {
        case R_wakey:
            Serial.println("Waking up device...");
            // Maybe signal a wake task or similar
            break;

        case R_toProc:
            Serial.print("Routing input to PROCESS: ");
            Serial.println(uinput.key, HEX);

            if (processInputQueue != nullptr) {
                if (xQueueSend(processInputQueue, &uinput, 0) != pdPASS) {
                    Serial.println("Process input queue full or unavailable.");
                }
            } else {
                Serial.println("No active process queue to receive input.");
            }
            break;

        case R_os:
            Serial.print("Routing input to OS: ");
            Serial.println(uinput.key, HEX);

            if (lockscreenQueue != nullptr) {
                if (xQueueSend(lockscreenQueue, &uinput, 0) != pdPASS) {
                    Serial.println("OS input queue full or unavailable.");
                }
            } else {
                Serial.println("No OS queue set.");
            }
            break;

        default:
            Serial.println("Unknown route target.");
            break;
    }
}





void PollEncoders() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 2) return;
  lastCheck = millis();

  // --- Encoder 0 (X axis) ---
  int cur0 = digitalRead(ENCODER0_CLK_PIN);
  if (cur0 != lastCLK_0 && cur0 == LOW) {
    bool cw = (digitalRead(ENCODER0_DT_PIN) != cur0);
    encoder0_Pos += cw ? 1 : -1;

    if (cw) {
      // Right
      if (!enc0_dir_right) {
        S_UserInput input = {key_right, true};
        RouteInput(input, currentinputTarget);
        enc0_dir_right = true;
      }
      // Release left if held
      if (enc0_dir_left) {
        S_UserInput input = {key_left, false};
        RouteInput(input, currentinputTarget);
        enc0_dir_left = false;
      }
    } else {
      // Left
      if (!enc0_dir_left) {
        S_UserInput input = {key_left, true};
        RouteInput(input, currentinputTarget);
        enc0_dir_left = true;
      }
      // Release right if held
      if (enc0_dir_right) {
        S_UserInput input = {key_right, false};
        RouteInput(input, currentinputTarget);
        enc0_dir_right = false;
      }
    }
  }
  lastCLK_0 = cur0;

  // On no movement, release any held
  if (lastCLK_0 == HIGH) {
    if (enc0_dir_right) { 
      S_UserInput input = {key_right, false};
      RouteInput(input, currentinputTarget); 
      enc0_dir_right = false; 
    }
    if (enc0_dir_left) { 
      S_UserInput input = {key_left, false};
      RouteInput(input, currentinputTarget); 
      enc0_dir_left = false; 
    }
  }

  // --- Encoder 1 (Y axis) ---
  int cur1 = digitalRead(ENCODER1_CLK_PIN);
  if (cur1 != lastCLK_1 && cur1 == LOW) {
    bool cw = (digitalRead(ENCODER1_DT_PIN) != cur1);
    encoder1_Pos += cw ? 1 : -1;

    if (cw) {
      // Down
      if (!enc1_dir_down) {
        S_UserInput input = {key_down, true};
        RouteInput(input, currentinputTarget);
        enc1_dir_down = true;
      }
      if (enc1_dir_up) {
        S_UserInput input = {key_up, false};
        RouteInput(input, currentinputTarget);
        enc1_dir_up = false;
      }
    } else {
      // Up
      if (!enc1_dir_up) {
        S_UserInput input = {key_up, true};
        RouteInput(input, currentinputTarget);
        enc1_dir_up = true;
      }
      if (enc1_dir_down) {
        S_UserInput input = {key_down, false};
        RouteInput(input, currentinputTarget);
        enc1_dir_down = false;
      }
    }
  }
  lastCLK_1 = cur1;

  // Release if not moving
  if (lastCLK_1 == HIGH) {
    if (enc1_dir_down) { 
      S_UserInput input = {key_down, false};
      RouteInput(input, currentinputTarget); 
      enc1_dir_down = false; 
    }
    if (enc1_dir_up) { 
      S_UserInput input = {key_up, false};
      RouteInput(input, currentinputTarget); 
      enc1_dir_up = false; 
    }
  }

  // --- Buttons ---
  if (button0Pressed) {
    button0Pressed = false;
    S_UserInput input = {key_enter, true};
    RouteInput(input, currentinputTarget);
  } else if (enc0_dir_right || enc0_dir_left) {
    // no change to user button
  } else {
    S_UserInput input = {key_enter, false};
    RouteInput(input, currentinputTarget);
  }

  if (button1Pressed) {
    button1Pressed = false;
    S_UserInput input = {key_back, true};
    RouteInput(input, currentinputTarget);
  } else {
    S_UserInput input = {key_back, false};
    RouteInput(input, currentinputTarget);
  }
}
/*
//unsure how i'll handle delays for this, i'll figure it out later
uint_16t HARDWAREKEYINPUTBITMASK=0x0000;

void keycombo_solver(uint_16 bitmask){ //function takes bitmask and returns the combo item (enum KEYCOMBO)
  //last 4 keys are stored in a uint_16. the range of keys we have is 0-5 (6 keys) fitting in 0xabcd instead of an array for less overhead. we poll this code really fast so that's why i'm doing bit twiddling
bitmask



}*/

