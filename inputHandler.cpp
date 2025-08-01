#include "Wiring.h"
#include "esp_sleep.h"
#include <Arduino.h>
#include "kernel.ino"
#include "driver/pulse_cnt.h"
#include "inputHandler.h"
#include "types.h"

 int16vect globalNavPos = {0, 0, 0};
 bool ShouldNavWrap = true;           // wrap or clamp
 int16vect Navlimits_ = {64, 64, 0};//xyz, just as example here init'd to 64 of each

// Actual struct definition (only here, not in header)
//typedef struct {pcnt_unit_handle_t unit;int16_t last_count;} encoder_state_t;

static encoder_state_t encoders[2];  // For X and Y axes
// PCNT unit assignments
#define ENC_UNIT_X 0
#define ENC_UNIT_Y 1

// Encoder channel assignments
#define PCNT_CHANNEL_X PCNT_CHANNEL_0
#define PCNT_CHANNEL_Y PCNT_CHANNEL_0  // Separate unit gets its own channel 0

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


//static encoder_state_t encoders[2];  // For X and Y axes
volatile int encoder0_Pos = 0;
volatile int encoder1_Pos = 0;

int lastCLK_0 = 1;
int lastCLK_1 = 1;

// To get encoder counts:
int16_t count_x, count_y;
//pcnt_unit_get_count(encoders[ENC_UNIT_X].unit, &count_x);
//pcnt_unit_get_count(encoders[ENC_UNIT_Y].unit, &count_y);

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

void SetupEncoders() {
pcnt_unit_config_t unit_config = {
    .low_limit = INT16_MIN,   // Must come first
    .high_limit = INT16_MAX,  // Then high limit
    .flags = {0}             // Initialize all flags to 0
};

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,  // 1µs glitch filter
    };

    // Channel configurations
    pcnt_chan_config_t chan_config = {
        .edge_gpio_num = ENCODER0_CLK_PIN,
        .level_gpio_num = ENCODER0_DT_PIN,
    };

    // Initialize X encoder
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &encoders[ENC_UNIT_X].unit));
    pcnt_channel_handle_t chan;
    ESP_ERROR_CHECK(pcnt_new_channel(encoders[ENC_UNIT_X].unit, &chan_config, &chan));
    
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(encoders[ENC_UNIT_X].unit, &filter_config));
    ESP_ERROR_CHECK(pcnt_unit_enable(encoders[ENC_UNIT_X].unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(encoders[ENC_UNIT_X].unit));
    ESP_ERROR_CHECK(pcnt_unit_start(encoders[ENC_UNIT_X].unit));

    // Initialize Y encoder (same config but different pins)
    chan_config.edge_gpio_num = ENCODER1_CLK_PIN;
    chan_config.level_gpio_num = ENCODER1_DT_PIN;
    
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &encoders[ENC_UNIT_Y].unit));
    ESP_ERROR_CHECK(pcnt_new_channel(encoders[ENC_UNIT_Y].unit, &chan_config, &chan));
    
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(encoders[ENC_UNIT_Y].unit, &filter_config));
    ESP_ERROR_CHECK(pcnt_unit_enable(encoders[ENC_UNIT_Y].unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(encoders[ENC_UNIT_Y].unit));
    ESP_ERROR_CHECK(pcnt_unit_start(encoders[ENC_UNIT_Y].unit));
}

void SetupHardwareInput() {
  // 1. First setup encoder hardware (PCNT)
  SetupEncoders();  // Modern pulse_cnt implementation
  
  // 2. Configure button pins
  const int buttonPins[] = {ENCODER0_SW_PIN, ENCODER1_SW_PIN};
  for(int pin : buttonPins) {
    pinMode(pin, INPUT_PULLUP);
  }

  // 3. Attach interrupts (debounced in hardware)
  attachInterrupt(digitalPinToInterrupt(ENCODER0_SW_PIN), handleButton0Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER1_SW_PIN), handleButton1Interrupt, FALLING);

  // 4. Sleep configuration (ONE of these, not both!)
  // Option A: EXT0 (single pin wake)
   esp_sleep_enable_ext0_wakeup((gpio_num_t)ENCODER0_SW_PIN, 0);
  
  // Option B: EXT1 (multiple pin wake) - RECOMMENDED
  esp_sleep_enable_ext1_wakeup(
      (1ULL << ENCODER0_SW_PIN) | (1ULL << ENCODER1_SW_PIN),
      ESP_EXT1_WAKEUP_ANY_LOW
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
 ENCODER0
 _CLK_PIN 
DT_PIN  
SW_PIN 

//y axis, back

 ENCODER1
 _CLK_PIN 
DT_PIN  
SW_PIN  
*/


// Fixed PollEncoders function for robust encoder and button handling
// Improvements:
// 1. Added per-button debounce timing
// 2. Removed unconditional key_enter release that caused stuck statePollEncoders
// 3. Unified edge detection: only send press/release on actual transitions
// 4. Simplified encoder "no movement" logic
// 5. Ensured auto-clear of transient flags outside input dispatch
//--6. fixed two pins and modified .h file
//7. 

// Constants for debounce intervals (ms)

static const uint8_t  BTN_DEBOUNCE_MS = 50;
// Encoder handling split from PollEncoders
// Constants for debounce intervals (ms)

//S_UserInput pollencoder(bool encoderid,){}

void PollEncoder(EncoderState *state, uint8_t clk_pin, uint8_t dt_pin, 
                 uint16_t key_cw, uint16_t key_ccw, HID_ROUTE_TARGET route) 
{
    uint32_t now = millis();
    uint8_t clk = digitalRead(clk_pin);
    
    // Only process if enough time has passed since last edge
    if ((now - state->last_edge_time) < ENC_COOLDOWN_MS) {
        return;
    }
    
    // Detect valid state change with debouncing
    if (clk != state->last_clk) {
        state->last_edge_time = now;
        
        // Only act on stable transitions after debounce period
        if ((now - state->last_edge_time) > ENC_DEBOUNCE_MS) {
            if (clk == LOW) {  // Active edge
                bool cw = (digitalRead(dt_pin) != clk);
                state->position += (cw ? 1 : -1);
                
                // Send key press
                RouteInput({cw ? key_cw : key_ccw, true}, route);
                state->dir_flag = true;
            }
            state->last_valid_clk = clk;
        }
        state->last_clk = clk;
    } 
    // Send key release after debounce
    else if (state->dir_flag && (clk == HIGH) && 
             (state->last_valid_clk == LOW) &&
             ((now - state->last_edge_time) > ENC_DEBOUNCE_MS)) 
    {
        RouteInput({key_cw, false}, route);
        RouteInput({key_ccw, false}, route);
        state->dir_flag = false;
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


/*
void PollEncoders() {
    
    unsigned long now = millis();
    if (now - lastEncCheck < ENC_DEBOUNCE_MS) return;
    lastEncCheck = now;
PollEncoder(&enc0_state, ENCODER0_CLK_PIN, ENCODER0_DT_PIN, 
               key_right, key_left, currentinputTarget);
    PollEncoder(&enc1_state, ENCODER1_CLK_PIN, ENCODER1_DT_PIN,
               key_down, key_up, currentinputTarget);
}*/
volatile bool encHTRGstate = 0; 
volatile bool encVTRGstate = 0;

void PollEncoders() {
    static uint32_t last_check = 0;
    uint32_t now = millis();
    
    if (now - last_check < 10) return;
    last_check = now;

    for (int i = 0; i < 2; i++) {
        int count; 
        ESP_ERROR_CHECK(pcnt_unit_get_count(encoders[i].unit, &count));
        
        int delta = count - encoders[i].last_count;
        if (delta != 0) {
            uint16_t key;
            bool direction = delta > 0;
            
            if (i == ENC_UNIT_X) {
                key = direction ? key_right : key_left;
                encHTRGstate = !encHTRGstate; // Toggle state for horizontal
                if(encHTRGstate) { // Only send on every other change
                    RouteInput({key, true}, currentinputTarget);
                    RouteInput({key, false}, currentinputTarget);
                }
            } else {
                key = direction ? key_down : key_up;
                encVTRGstate = !encVTRGstate; // Toggle state for vertical
                if(encVTRGstate) { // Only send on every other change
                    RouteInput({key, true}, currentinputTarget);
                    RouteInput({key, false}, currentinputTarget);
                }
            }
            
            encoders[i].last_count = count;
        }
    }
}
/*
//unsure how i'll handle delays for this, i'll figure it out later
uint_16t HARDWAREKEYINPUTBITMASK=0x0000;

void keycombo_solver(uint_16 bitmask){ //function takes bitmask and returns the combo item (enum KEYCOMBO)
  //last 4 keys are stored in a uint_16. the range of keys we have is 0-5 (6 keys) fitting in 0xabcd instead of an array for less overhead. we poll this code really fast so that's why i'm doing bit twiddling
bitmask



}*/
