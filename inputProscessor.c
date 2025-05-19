#include "Wiring.h"
#include "esp_sleep.h"
//remember:
/* //x axis,sel
#define ENCODER0_CLK_PIN 42
#define ENCODER0_DT_PIN  14
#define ENCODER0_SW_PIN  7

//y axis, back
//FIX IT, WE HAVEN'T PUT THIS IN YET
#define ENCODER1_CLK_PIN //21
#define ENCODER1_DT_PIN  //19
#define ENCODER1_SW_PIN  //18

*/


//volatile bool button0Pressed = false;
//volatile bool button1Pressed = false;

//volatile int encoder0_Pos = 0;
//volatile int encoder1_Pos = 0;

int lastCLK_0 = 1;
int lastCLK_1 = 1;

void IRAM_ATTR handleButton0Interrupt() {
  button0Pressed = true;
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
      ESP_EXT1_WAKEUP_ALL_LOW
  );

}



//
struct S_UserInput {
    uint16_t key;
    bool isDown;
   };

enum inputDeviceType{
  mouse, //anything with an xy pos and a clicky
  keyboard, //wasd. fuck you
  device //the two dials
};

enum HID_ROUTE_TARGET{
  wakey, //the device is in sleep mode and needs to be woke
  toProc, //to focused proscess
  os //route to the os, not anything else.
};


constexpr bool AllowSimultaneousKeypress = true;


constexpr uint16_t key_enter 0x23CE; // ⏎
constexpr uint16_t key_back 0x232B;  // ⌫
constexpr uint16_t key_up 0x2191;// ↑
constexpr uint16_t key_down 0x2193; // ↓
constexpr uint16_t key_left 0x2190; // ←
constexpr uint16_t key_right 0x2192; // →

bool enc0_dir_right = false;
bool enc0_dir_left  = false;
bool enc1_dir_up    = false;
bool enc1_dir_down  = false;

//replace with map
HID_ROUTE_TARGET currentinputTarget = os;//route to




void RouteInput(S_UserInput uinput, HID_ROUTE_TARGET uout) {
  switch (uout) {
    case wakey:
      Serial.println("Waking up device...");
      




      break;

    case toProc:
      Serial.print("Send input to process: ");
      Serial.println(uinput.key, HEX);
      // TODO: send to app
      break;

    case os:
      Serial.print("Send input to OS: ");
      Serial.println(uinput.key, HEX);
      // TODO: route globally
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
        RouteInput({ key_right, true }, currentinputTarget);
        enc0_dir_right = true;
      }
      // Release left if held
      if (enc0_dir_left) {
        RouteInput({ key_left, false }, currentinputTarget);
        enc0_dir_left = false;
      }
    } else {
      // Left
      if (!enc0_dir_left) {
        RouteInput({ key_left, true }, currentinputTarget);
        enc0_dir_left = true;
      }
      // Release right if held
      if (enc0_dir_right) {
        RouteInput({ key_right, false }, currentinputTarget);
        enc0_dir_right = false;
      }
    }
  }
  lastCLK_0 = cur0;

  // On no movement, release any held
  if (lastCLK_0 == HIGH) {
    if (enc0_dir_right) { RouteInput({ key_right, false }, currentinputTarget); enc0_dir_right = false; }
    if (enc0_dir_left ) { RouteInput({ key_left , false }, currentinputTarget); enc0_dir_left  = false; }
  }

  // --- Encoder 1 (Y axis) ---
  int cur1 = digitalRead(ENCODER1_CLK_PIN);
  if (cur1 != lastCLK_1 && cur1 == LOW) {
    bool cw = (digitalRead(ENCODER1_DT_PIN) != cur1);
    encoder1_Pos += cw ? 1 : -1;

    if (cw) {
      // Down
      if (!enc1_dir_down) {
        RouteInput({ key_down, true }, currentinputTarget);
        enc1_dir_down = true;
      }
      if (enc1_dir_up) {
        RouteInput({ key_up, false }, currentinputTarget);
        enc1_dir_up = false;
      }
    } else {
      // Up
      if (!enc1_dir_up) {
        RouteInput({ key_up, true }, currentinputTarget);
        enc1_dir_up = true;
      }
      if (enc1_dir_down) {
        RouteInput({ key_down, false }, currentinputTarget);
        enc1_dir_down = false;
      }
    }
  }
  lastCLK_1 = cur1;

  // Release if not moving
  if (lastCLK_1 == HIGH) {
    if (enc1_dir_down) { RouteInput({ key_down, false }, currentinputTarget); enc1_dir_down = false; }
    if (enc1_dir_up  ) { RouteInput({ key_up  , false }, currentinputTarget); enc1_dir_up   = false; }
  }

  // --- Buttons ---
  if (button0Pressed) {
    button0Pressed = false;
    RouteInput({ key_enter, true }, currentinputTarget);
  } else if (enc0_dir_right || enc0_dir_left) {
    // no change to user button
  } else {
    RouteInput({ key_enter, false }, currentinputTarget);
  }

  if (button1Pressed) {
    button1Pressed = false;
    RouteInput({ key_back, true }, currentinputTarget);
  } else {
    RouteInput({ key_back, false }, currentinputTarget);
  }
}




//hardware input->key mapping->user input struct->kb statemachine->focused os proscess/wake device/emergency back




