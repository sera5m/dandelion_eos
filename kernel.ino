#ifndef KERNEL_H
#define KERNEL_H


// Include necessary headers
#include <memory>
#include <string>
#include <mutex>
#include <queue>
#include <atomic>
#include <functional>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "inputHandler.h" //this kernel should take the key inputs and send to focused proscess or os. the input proscessor handles wake states 
// Forward declarations




//todo remove this dead code
class OSProcess;




class OSProcess : public std::enable_shared_from_this<OSProcess> {
public:
    using ProcessCallback = std::function<void(OSProcess*)>;
    
    struct KPROC_Config {
        std::string name;
        ProcessCallback callback;
        uint32_t stackSize = 4096;  // In words (not bytes) for FreeRTOS
        UBaseType_t priority = tskIDLE_PRIORITY + 1;
        bool pinToCore = false;     // ESP32-specific
        int coreId = 0;            // ESP32-specific
    };

    // Factory method
    template<typename T, typename... Args>
    [[nodiscard]] static std::shared_ptr<T> create(Args&&... args) {
        static_assert(std::is_base_of_v<OSProcess, T>, 
                     "Must derive from OSProcess");
        
        struct EnableMakeShared : public T {
            EnableMakeShared(Args&&... args) : T(std::forward<Args>(args)...) {}
        };
        
        auto proc = std::make_shared<EnableMakeShared>(std::forward<Args>(args)...);
        if (!proc->start()) {
            return nullptr;  // Task creation failed
        }
        return proc;
    }

    virtual ~OSProcess() {
        if (taskHandle != nullptr) {
            vTaskDelete(taskHandle);  // Clean up the FreeRTOS task
            taskHandle = nullptr;
        }
    }

    // Non-pure virtual (default empty implementation)
    virtual void onInput(const S_UserInput& input) {}

protected:
    explicit OSProcess(const KPROC_Config& cfg) : config(cfg) {}

    bool start() {
        // Static wrapper for FreeRTOS task function
        auto taskFunc = [](void* arg) {
            OSProcess* self = static_cast<OSProcess*>(arg);
            if (self->config.callback) {
                self->config.callback(self);
            }
            vTaskDelete(nullptr);  // Self-delete if callback returns
        };

        BaseType_t result;
        if (config.pinToCore) {
            // ESP32-specific pinned-to-core version
            result = xTaskCreatePinnedToCore(
                taskFunc,
                config.name.c_str(),
                config.stackSize,
                this,
                config.priority,
                &taskHandle,
                config.coreId
            );
        } else {
            // Standard FreeRTOS version
            result = xTaskCreate(
                taskFunc,
                config.name.c_str(),
                config.stackSize,
                this,
                config.priority,
                &taskHandle
            );
        }

        return (result == pdPASS);
    }

private:
    KPROC_Config config;
    TaskHandle_t taskHandle = nullptr;
};

 static inline std::weak_ptr<OSProcess> focusedProcess; //ref for os




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
//float FastNormalizeAngle_radians(float angle) {
    // Normalize angle to be within [-2*PI, 2*PI] (radians)
  //  return fmod(angle, 2 * M_PI);  // Directly return the remainder (which can be negative)
//}
  //figured this out in matlab for uni and found that it'd be useful here, lol


//faster approximations for trig functions for 3d rendering- all math in radians unless explicitly specified. 

//lookup tables for full degrees, use lerp for half degrees-BEST USED FOR RENDERING! DO NOT USE IN YOUR CALCULATORS! THIS IS 2 DIGIT PRESCISION!
//todo: include the lut if i really need but i don't think i need it so i'll do this later(never)


//various macros

//note to self: when i did this on feb 6 2025 at midnight, i had to learn macros.--i was fucking watching minecraft shi on the right screen
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

/*
// Function to create a new rate limit counter
int createRateLimit() {
    if (rate_limit_count < MAX_RATE_LIMITS) {
        rate_limits[rate_limit_count].unlock_time = 0;
        return rate_limit_count++;  // Return current ID, then increment
    }
    return -1;  // Return -1 if no more counters can be created
}*/

// Get current time (replace with your actual function)


/*
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


*/







#endif // KERNEL_H 
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


/////////////////////////////////////////////end of rate limit macro



//toggle state macro

#define TOGGLE_STATE(state_var) (state_var = !(state_var))

//to use:

bool light_on = false;
TOGGLE_STATE(light_on);
Serial.println(light_on ? "Light ON" : "Light OFF");








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




//****************************************************** do stuff a bunch of times-repeatx

 //do stuff with no delay x times
#define REPEAT_X(times, on_exec)               \
    do {                                       \
        for (int i = 0; i < (times); i++) {    \
            on_exec;                           \
        }                                      \
    } while (0)

//example

//WARNING: static variables may be in here. check these-but should be changed

 //do stuff with delay x times
#define REPEAT_X_WITH_DELAY(times, interval, on_exec)   \
     int T__repeat_count = 0;                        \
     unsigned long T__last_time = 0;                 \
    if (T__repeat_count < (times)) {                       \
        if (millis() - T__last_time >= (interval)) {       \
            T__last_time = millis();                       \
            on_exec;                                      \
            T__repeat_count++;                             \
        }                                                 \
    }

//example


   // REPEAT_X_WITH_DELAY(5, 1000, Serial.println("Non-blocking Hello!"));

/*


/*

//ultrahigh speed repeat with delay-uses microseconds
#define REPEAT_X_WITH_US_DELAY(times, interval_us, on_exec)  \
     int T__repeat_count_us = 0;                          \
     unsigned long T__last_time_us = 0;                   \
    if (T__repeat_count_us < (times)) {                         \
        if (micros() - T__last_time_us >= (interval_us)) {      \
            T__last_time_us = micros();                         \
            on_exec;                                           \
            T__repeat_count_us++;                               \
        }                                                      \
    }

 //use:

    //REPEAT_X_WITH_US_DELAY(3, 500, Serial.println("Microsecond Hello!"));



/*
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

 //how to use




*/
