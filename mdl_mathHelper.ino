#ifndef MDL_MATH_HELPER_H
#define MDL_MATH_HELPER_H

//include the other stuff we need
#include "lillypad_renderer.ino" //because task creation has window req

#include <memory>
#include <string>
#include <mutex>

//this module is made to provide arduino the half dozen math things it doesn't natively support because god fucking help me why woulnd't we have basics like vector 3? 
//noooooo that would bne too fucking hard wouldn't it - sera5m 12/1/2024
//update: it's 3 days later. IT GETS WORSE. WAY WORSE. ASM HELL. I'M IN HELLLLLLLLLLL
//note to past self: why did you even try inline asm? just make better code. i mean i'm not gonna do it. oh wait, i have to
//shut tufuick up

//notes on hardware of the esp32:
//fpu: the fpu is absolute garbage. like. faster than me, but still MISERABLY bad for a fpu. (functionally useless proscessor) i'm a person with a soul. and i deserve to not suffer like this. i mean i've had worse life experiences but....
//the cpu or fpu doesn't support division natively. why? that's like. the most normal thing to have. 




//forward class declare so no bugs you vill eat ze bugs
class OSProscess; 
class OSProcessHandlerService; 
//i am this fucking close


 //yummy yummy import user input
//fuck you, i'm being lazy and moving user input into here to not deal with stupid fucking errors




#define MAX_KEYS 6  // Maximum number of keys to track at once. device has 6 physical keys fuck off

bool AllowSimotaneousKeypress=true; //i'll need this later-does nothing yet. also how to spell????


enum hardwareButtons { //the 6 physical buttons the shit has
  Up,
  Down,
  Left,
  Right,
  Enter,
  Back
}; //p0-p5

constexpr uint16_t BUTTON_MAPPING[] = { //os-defined mapping to map said buttons to unicode
  0x2191, // ↑ Up Arrow
  0x2193, // ↓ Down Arrow
  0x2190, // ← Left Arrow
  0x2192, // → Right Arrow
  0x23CE, // ⏎ enter/sel
  0x232B  // ⌫ Back/backspace
};


//  store button states
bool buttonStates[6] = {false}; //you should use a bitmask for this and change to physicalbuttonstates




volatile bool interruptFlag = false; //allow interupt use? i don't remember the purpose

// Interrupt Service Routine (ISR)
void IRAM_ATTR handleInterrupt() {
  interruptFlag = true; // Indicate that a button press occurred
}


// UserInput struct
struct UserInput {
    uint8_t type;     // Device Type (e.g., Keyboard, Mouse, some dickhead on their phone)
    uint8_t device;   // Specific Device ID. 000000000 for the buttons on the physical device
    uint16_t key;     // Unicode key value
    bool isDown;      // True if pressed, False if released. no fucking shit
    uint16_t MS_HeldDown; //how long it was held down before being released [max 655s=11 mins. if they hold it longer, they're fucking insane]
};

// Handle button presses and map them to Unicode
void OnButtonPush(bool isDown, hardwareButtons button) {
  UserInput input;
  input.type = 0xF0; // Mark as physical input from the actual watch buttons-note: doccument this
  input.device = 0;  // Internal buttons
  input.key = BUTTON_MAPPING[button]; //buttons are keys. kina
  input.isDown = isDown; //duhhhhhhhhhhhhh
 //
  OnUserInput(input); // what does this do
}

void PollButtons() {
  for (int i = 0; i < 6; i++) {
    bool newState = (pcf.digitalRead(i) == LOW); // Active LOW
    if (newState != buttonStates[i]) {
      buttonStates[i] = newState;
      // Trigger interrupt for input handling
      handleInterrupt(); // set the interrupt flag
    }
  }
}








/*
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣠⣤⣤⣤⣤⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⣠⡶⡿⢿⣿⣛⣟⣿⡿⢿⢿⣷⣦⡀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⢰⣯⣷⣿⣿⣿⢟⠃⢿⣟⣿⣿⣾⣷⣽⣺⢆⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⢸⣿⢿⣾⢧⣏⡴⠀⠈⢿⣘⣿⢿⣿⣿⣿⣿⡆⠀⠀⠀
⠀⠀⠀⠀⠀⠀⢹⣿⢠⡶⠒⢶⠀⠀⣠⠒⠒⠢⡀⢿⣿⣿⣿⡇⠀⠀⠀
⠀⠀⠀⠀⠀⠀⣿⣿⠸⣄⣠⡾⠀⠀⠻⣀⣀⡼⠁⢸⣿⣿⣿⣿⠀⠀⠀
⠀⠀⠀⠀⠀⠀⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⠀⠀
⠀⠀⠀⠀⠀⢰⣿⣿⠀⠀⠀⡔⠢⠤⠔⠒⢄⠀⠀⢸⣿⣿⣿⣿⡇⠀⠀
⠀⠀⠀⠀⠀⢸⣿⣿⣄⠀⠸⡀⠀⠀⠀⠀⢀⡇⠠⣸⣿⣿⣿⣿⡇⠀⠀
⠀⠀⠀⠀⠀⢸⣿⣿⣿⣷⣦⣮⣉⢉⠉⠩⠄⢴⣾⣿⣿⣿⣿⡇⠀⠀⠀
⠀⠀⠀⠀⠀⢸⣿⣿⢻⣿⣟⢟⡁⠀⠀⠀⠀⢇⠻⣿⣿⣿⣿⣿⠀⠀⠀
⠀⠀⠀⠀⠀⢸⠿⣿⡈⠋⠀⠀⡇⠀⠀⠀⢰⠃⢠⣿⡟⣿⣿⢻⠀⠀⠀
⠀⠀⠀⠀⠀⠸⡆⠛⠇⢀⡀⠀⡇⠀⠀⡞⠀⠀⣸⠟⡊⠁⠚⠌⠀⠀⠀
⠀⠀⠀⠀⠀⠀⡍⠨⠊⣒⠴⠀⡇⡴⠋⡋⢐⠐⠅⡀⠐⢠⠕⠂⢂⠀
        ohmagah 
*/

//miniproscess for freerots.
//think of this like a setup layer for program proscesses as opposed to using a direct freerots. 
//NOTE: NOT PINNED TO CORE BY DEFAULT UNLESS FLAG


//to check: does focus remove window? it should not. a task being a background task should do that instead

class OSProcess: 
public: std::enable_shared_from_this<OSProscess>{ //std enable exposes os proscess refs for ext ref

    struct OSPConfig {
        std::string name;
        bool create_window = false; //auto make win?
        WindowCfg window_cfg; //win cfg
        bool pin_to_core = false; 
        int core_id = 0; //only relavent forpintocore
        uint32_t stack_size = 4096;  
        UBaseType_t priority = tskIDLE_PRIORITY + 1; //todo: is this really this high priority?
        bool start_focused = false; 
        bool isBackground=false; //new tasks not background by defualt-don't focus and have no window. must not be backgroudn to take input
        //todo: default define background tasks? unfocused tasks aren't automatically background ones, they can still be on screen
        
    };

    static std::shared_ptr<OSProcess> create(const OSPConfig& cfg) { 
        auto proc = std::make_shared<OSProcess>(cfg);
        OSProcessHandlerService::registerProcess(proc);
        if (cfg.start_focused) {
            OSProcessHandlerService::setFocused(proc);
        }
        return proc;
    }

    void start(bool auto_focus = true) {
        if (!OSProcessHandlerService::hasEnoughMemory(config.stack_size)) return; //do we have memory? if not, fuck you,no proscess

        if (!task_handle) { // Step 2: Check if the task has already been created (i.e., task_handle is not initialized)
            xTaskCreatePinnedToCore(taskRouter,OSPConfig.name.c_str(),OSPConfig.stack_size,this,OSPConfig.priority,&task_handle,OSPConfig.pin_to_core ?OSPConfig.core_id : tskNO_AFFINITY); //create new tas w conf
            if (auto_focus){ OSProcessHandlerService::setFocused(shared_from_this()); /*if the task is set to autofocus, notify the os handler service to focus in */ 
            }//ifautofocus
        } //thandle
      
      //call create window here to create and start it, but only if bool createwindow is true, and windowcfg is valid
        //TODO!!!!!!!!!!!!! CREATE THE FUCKING DINWODW NOW BTICH
      
    }//voidstart


void stop() {
    if (task_handle) {
        vTaskDelete(task_handle);
        task_handle = nullptr;
    }
    auto self = shared_from_this(); // Ensure we have a strong reference before unregistering
    OSProcessHandlerService::unregisterProcess(self);
    // TODO: Delete associated window if it exists
}



  bool setProcBackground(bool isBG){ //needs to be able to be remotely called too 
  //1. delete and dereg the current window from the lillypad graphics thing
  //2. set unfocused
  //3. tell the os proscess handler this one is in the background now
  //4. throttle update rate (once we impliment update rates)
  }

  void TakeUsrInput(const UserInput& input){ //take the user input and do that
  //no logic yet-needs to take the user input intertupt passed to it from the os proscess manager
  }//put it in a lambadia or something that the proscess can handle with it's own custom code-because i havea to remember that the os priscess is just a fucking wrapper!
//user input should really be routed to whatever logic the user's implimented




private:
    explicit OSProcess(const OSPConfig& cfg) : OSPConfig(cfg), execution_code([]{}) {}


    static void taskRouter(void* param) {
        auto* proc = static_cast<OSProcess*>(param);
        if (proc->execution_code) {
            proc->execution_code();
        }
        proc->stop();
    }

   OSPConfig OSPConfig; 
    TaskHandle_t task_handle = nullptr;
    std::function<void()> execution_code;
};

class OSProcessHandlerService {
public:


    static void registerProcess(std::shared_ptr<OSProcess> proc) {
        std::lock_guard<std::mutex> lock(process_mutex);
        processes.emplace_back(proc);
    }

    static void unregisterProcess(std::shared_ptr<OSProcess> proc) {
        std::lock_guard<std::mutex> lock(process_mutex);
        processes.erase(
            std::remove_if(processes.begin(), processes.end(),
                [&proc](const auto& p) { return p.lock() == proc; }),
            processes.end()
        );
    }

    static bool hasEnoughMemory(uint32_t stack_size) {
        return xPortGetFreeHeapSize() > (stack_size + 64); //mem and fudging
    }

    static void setFocused(std::shared_ptr<OSProcess> proc) { //route input to here now
        std::lock_guard<std::mutex> lock(process_mutex);
        focused_process = proc;
    }


// Function to send input to the focused process. 
void OnUserInput(const UserInput& input) { //todo: not sure how to use this functionality, but at least the pointers shouldn't do anything weird
    std::lock_guard<std::mutex> lock(process_mutex);
    auto proc = focused_process.lock(); // Convert weak_ptr to shared_ptr
    if (proc) { 
        proc->TakeUsrInput(input); // Send input to the focused process
    }
}


private:
    static inline std::vector<std::weak_ptr<OSProcess>> processes;
    static inline std::weak_ptr<OSProcess> focused_process;
    static inline std::mutex process_mutex;
};


struct Vector3 { //todo: fix it. not sure if it works right but whatever i guess
    int16_t x; // Scaled to 2 decimal places
    int16_t y; // Scaled to 2 decimal places
    int16_t z; // Scaled to 2 decimal places

};




struct DVector3 { //vector3 if it was doubles
    double x;
    double y;
    double z;
};

struct Vector4 { //vec4-for when you need 1 more dimension
    float x;
    float y;
    float z;
    float w;
};

struct DVector4 { //vector4 if it was doubles
    double x;
    double y;
    double z;
    double w;
};

//put function to get len of ANY vector here. eg it goes through, adds em and sqrt and shit, if double then break internally and use double logic

//put do n here

//normalize a float angle, but fast. RADIANS ONLY
float FastNormalizeAngle_radians(float angle) {
    // Normalize angle to be within [-2*PI, 2*PI] (radians)
    return fmod(angle, 2 * M_PI);  // Directly return the remainder (which can be negative)
}
  //figured this out in matlab for uni and found that it'd be useful here, lol


//faster approximations for trig functions for 3d rendering- all math in radians unless explicitly specified. 

//lookup tables for full degrees, use lerp for half degrees-BEST USED FOR RENDERING! DO NOT USE IN YOUR CALCULATORS! THIS IS 2 DIGIT PRESCISION!
//todo: include the lut if i really need but i don't think i need it so i'll do this later(never)


//various macros


//note to self: when i did this on feb 6 2025 at midnight, i had to learn macros.
//here's how they work so i remember. macros in c just kina replace the code they're put in with their own code so they're proceedurally swapped or wahtever
//don't repeat code. use macros. even though they're weird to make. no runtime overhead
//preproscessor straight up replaces the thing where you call the macro with the macro code when it's put into the computer
//blocks of code behave like single statements
//warning: no type safety and harder to debug. debuggers see expanded ciode

//WARNING: DO NOT FUCKING PUT WHITESPACE AFTAER MACRO SLASHES. IT WILL BREAK IT.. THE SLASH MUST BE THE LAST CHAR ON THE LINE.
//SHOULD show up as blue not white normally in the arduino ide


//******************************************************rate limit macro

//info:
//can autogen ids
//memory:
//ASSUMED 4 bytes per counter (unsigned long unlock_time)
//warning:each rate limit called increases ram use! by a few bytes


// Struct for rate limit counters
typedef struct {
    unsigned long unlock_time;
} RateLimitCounter;

// Dynamic array of counters. size can be reconfigured
#define MAX_RATE_LIMITS 2048 
static RateLimitCounter rate_limits[MAX_RATE_LIMITS];
static int rate_limit_count = 0;  // Tracks the number of rate limiters created

// Function to create a new rate limit counter
int createRateLimit() {
    if (rate_limit_count < MAX_RATE_LIMITS) {
        rate_limits[rate_limit_count].unlock_time = 0;
        return rate_limit_count++;  // Return current ID, then increment
    }
    return -1;  // Return -1 if no more counters can be created
}

// Get current time (replace with your actual function)
unsigned long getUnixTime() {
    return millis() / 1000;  // Example using millis() for simplicity
}


// Deregister a rate limit counter
void deleteRateLimit(int id) {
    if (id >= 0 && id < rate_limit_count) {
        // Shift all counters after 'id' to fill the gap
        for (int i = id; i < rate_limit_count - 1; i++) {
            rate_limits[i] = rate_limits[i + 1];
        }
        rate_limit_count--;
    }
}



  //oughhhh it's a function to limit the rate you can enter it
#define RATE_LIMIT(id, interval_ms, on_success, not_unlocked_yet)       \
    do {                                                                \
        if ((id) >= 0 && (id) < MAX_RATE_LIMITS) {                      \
            unsigned long current_time = getUnixTime();                 \
            if (current_time >= rate_limits[(id)].unlock_time) {        \
                on_success;                                             \
                rate_limits[(id)].unlock_time = current_time + (interval_ms); \
            } else {                                                   \
                not_unlocked_yet;                                       \
            }                                                          \
        }                                                              \
    } while (0)

//to use, copy the following and impliment with whatever logic in the main code or whatever

/*

//assign an id automatically
static int button_press_id = -1; 

    // Initialize rate limiter once
    if (button_press_id == -1) {
        button_press_id = createRateLimit(); //create a rate limit id auto assigned to this
    }


//use of rate limit gate:

    RATE_LIMIT(button_press_id, 5000, //set up using the macro ref id
        {
            Serial.println("Unlocked: Executing code.");
        },
        {
            Serial.println("Not unlocked yet.");
        }
    );
//should? be the same as my do and lock for macro
*/


/////////////////////////////////////////////end of rate limit macro



//toggle state macro

#define TOGGLE_STATE(state_var) (state_var = !(state_var))

//to use:
/*
bool light_on = false;
TOGGLE_STATE(light_on);
Serial.println(light_on ? "Light ON" : "Light OFF");

*/






//counter. you enter and run this counter for whatever the fuck you need to count how many times you enter it. because you gotta count things

#define CREATE_COUNTER() createCounter()
#define INCREMENT_COUNTER(id, on_exec)         \
    do {                                       \
        if (id >= 0 && id < counter_count) {   \
            counters[id].value++;              \
            on_exec;                           \
        }                                      \
    } while (0)

#define RESET_COUNTER(id, on_reset)            \
    do {                                       \
        if (id >= 0 && id < counter_count) {   \
            counters[id].value = 0;            \
            on_reset;                          \
        }                                      \
    } while (0)

// Struct for counter
typedef struct {
    int value;
} Counter;

Counter counters[32];    // Max 32 counters
int counter_count = 0;

// Create new counter
int createCounter() {
    if (counter_count < 32) {
        counters[counter_count].value = 0;
        return counter_count++;
    }
    return -1; // Failed to create
}

//how to use
/*
int myCounter = CREATE_COUNTER();

INCREMENT_COUNTER(myCounter, {
    Serial.print("Counter: ");
    Serial.println(counters[myCounter].value);
});

RESET_COUNTER(myCounter, {
    Serial.println("Counter Reset!");
});

*/


//****************************************************** do stuff a bunch of times-repeatx

 //do stuff with no delay x times
#define REPEAT_X(times, on_exec)               \
    do {                                       \
        for (int i = 0; i < (times); i++) {    \
            on_exec;                           \
        }                                      \
    } while (0)

//example
/*
REPEAT_X(5, {
    Serial.println("Hello!");
});
*/

 //do stuff with delay x times
#define REPEAT_X_WITH_DELAY(times, interval, on_exec)   \
    static int __repeat_count = 0;                        \
    static unsigned long __last_time = 0;                 \
    if (__repeat_count < (times)) {                       \
        if (millis() - __last_time >= (interval)) {       \
            __last_time = millis();                       \
            on_exec;                                      \
            __repeat_count++;                             \
        }                                                 \
    }

//example
/*

    REPEAT_X_WITH_DELAY(5, 1000, Serial.println("Non-blocking Hello!"));
}

*/



//ultrahigh speed repeat with delay-uses microseconds
#define REPEAT_X_WITH_US_DELAY(times, interval_us, on_exec)  \
    static int __repeat_count_us = 0;                          \
    static unsigned long __last_time_us = 0;                   \
    if (__repeat_count_us < (times)) {                         \
        if (micros() - __last_time_us >= (interval_us)) {      \
            __last_time_us = micros();                         \
            on_exec;                                           \
            __repeat_count_us++;                               \
        }                                                      \
    }

/* //use:

    REPEAT_X_WITH_US_DELAY(3, 500, Serial.println("Microsecond Hello!"));


*/

//uhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh

//NAMING FUCKED UP HERE FIX IT
typedef struct loopwhiletimercreateinfo {
    unsigned long start_time;
    unsigned long duration;
    bool active;
    bool paused;
    unsigned long pause_time; // Time when the timer was paused
} LoopWhileTimer;

LoopWhileTimer loopWhileTimers[32]; // Consistent naming
int timer_count = 0;

int createLoopWhileTimer(unsigned long duration_ms) {
    if (timer_count < 32) {
        loopWhileTimers[timer_count] = {millis(), duration_ms, true, false, 0};
        return timer_count++;
    }
    return -1; // No more timers available
}

// Execute `on_tick` every `tick_interval` while the timer is active
#define LOOP_WHILE_TIMER(timer_id, tick_interval, on_tick)             \
    do {                                                               \
        if ((timer_id) >= 0 && (timer_id) < timer_count) {             \
            LoopWhileTimer *t = &loopWhileTimers[timer_id];            \
            if (t->active && !t->paused && millis() - t->start_time < t->duration) { \
                static unsigned long last_tick_##timer_id = 0;         \
                if (millis() - last_tick_##timer_id >= (tick_interval)) { \
                    on_tick;                                           \
                    last_tick_##timer_id = millis();                   \
                }                                                      \
            }                                                          \
        }                                                              \
    } while (0)

#define PAUSE_TIMER(timer_id)                                          \
    if ((timer_id) >= 0 && (timer_id) < timer_count) {                 \
        loopWhileTimers[timer_id].paused = true;                       \
        loopWhileTimers[timer_id].pause_time = millis();               \
    }

#define RESUME_TIMER(timer_id)                                         \
    if ((timer_id) >= 0 && (timer_id) < timer_count) {                 \
        loopWhileTimers[timer_id].paused = false;                      \
        loopWhileTimers[timer_id].start_time += millis() - loopWhileTimers[timer_id].pause_time; \
    }

#define RESET_TIMER(timer_id)                                          \
    if ((timer_id) >= 0 && (timer_id) < timer_count) {                 \
        loopWhileTimers[timer_id].start_time = millis();               \
    }

#define DELETE_TIMER(timer_id)                                         \
    if ((timer_id) >= 0 && (timer_id) < timer_count) {                 \
        loopWhileTimers[timer_id].active = false;                      \
    }

#define SKIP_TO_LAST_TICK(timer_id)                                    \
    if ((timer_id) >= 0 && (timer_id) < timer_count) {                 \
        loopWhileTimers[timer_id].start_time = millis() - loopWhileTimers[timer_id].duration; \
    }

/* //how to use
WARNING CREEATE A USER GUIDE. I DON'T REMEMBER HOW IT WORKS
*/



#endif