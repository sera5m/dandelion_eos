#ifndef MDL_MATH_HELPER_H
#define MDL_MATH_HELPER_H
//this module is made to provide arduino the half dozen math things it doesn't natively support because god fucking help me why woulnd't we have basics like vector 3? 
//noooooo that would bne too fucking hard wouldn't it - sera5m 12/1/2024
//update: it's 3 days later. IT GETS WORSE. WAY WORSE. ASM HELL. I'M IN HELLLLLLLLLLL


//notes on hardware of the esp32:
//fpu: the fpu is absolute garbage. like. faster than me, but still MISERABLY bad for a fpu. (functionally useless proscessor) i'm a person with a soul. and i deserve to not suffer like this. i mean i've had worse life experiences but....
//the cpu or fpu doesn't support division natively. why? that's like. the most normal thing to have. 
//this is wildly different than normal asm. fuck with this at your own peril, AND PERIL YOU WILL GET. 




//miniproscess for freerots.
//think of this like a setup layer for program proscesses as opposed to using a direct freerots. 
//NOTE: NOT PINNED TO CORE BY DEFAULT UNLESS FLAG


class OSProcess {
public:
    struct Config {
        std::string name;
        bool create_window = false;
        WindowCfg window_cfg;
        bool pin_to_core = false;
        int core_id = 0;
        uint16_t stack_size = 4096;  // More realistic minimum for C++ tasks
        UBaseType_t priority = tskIDLE_PRIORITY + 1;
        bool start_focused = false;  // New: Default focus state
    };

    // Factory method with background awareness
    static std::shared_ptr<OSProcess> create(const Config& cfg) {
        auto proc = std::make_shared<OSProcess>(cfg);
        std::lock_guard<std::mutex> lock(process_mutex);
        
        // Add to process registry
        auto& registry = processes();
        registry.erase(
            std::remove_if(registry.begin(), registry.end(),
                [](const auto& p) { return p.expired(); }),
            registry.end()
        );
        registry.emplace_back(proc);
        
        // Set focus if configured
        if(cfg.start_focused) setFocused(proc);
        
        return proc;
    }

    // Improved input handling with long-press detection
    static void handleGlobalInput(const UserInput& input) {
        static std::chrono::steady_clock::time_point back_press_time;
        static constexpr auto long_press_duration = std::chrono::seconds(10);

        std::lock_guard<std::mutex> lock(process_mutex);
        
        if(input.key == 0x232B) { // Back key
            if(input.isDown) {
                back_press_time = std::chrono::steady_clock::now();
            } else {
                auto duration = std::chrono::steady_clock::now() - back_press_time;
                if(duration > long_press_duration) {
                    emergencyReturnToMain();
                    return;
                }
            }
        }

        if(auto focused = focused_process.lock()) {
            focused->handleInput(input);
        }
    }

    // Enhanced process control
    void start(bool auto_focus = true) {
        if(!task_handle && execution_code) {
            xTaskCreatePinnedToCore(
                taskRouter, 
                config.name.c_str(),
                config.stack_size,
                this,
                config.priority,
                &task_handle,
                config.pin_to_core ? config.core_id : tskNO_AFFINITY
            );
            
            if(auto_focus) setFocused(shared_from_this());
        }
    }

    void stop() {
        if(task_handle) {
            vTaskDelete(task_handle);
            task_handle = nullptr;
            
            // Automatically cleanup window
            if(window) {
                unregisterWindow(window.get());
                window.reset();
            }
            
            // Remove from registry
            std::lock_guard<std::mutex> lock(process_mutex);
            auto& registry = processes();
            registry.erase(
                std::remove_if(registry.begin(), registry.end(),
                    [this](const auto& p) { return p.lock().get() == this; }),
                registry.end()
            );
        }
    }

    // Window management
    void createWindow() {
        if(!window) {
            window = std::make_unique<Window>(config.name, config.window_cfg);
            registerWindow(window.get());
        }
    }

    void destroyWindow() {
        if(window) {
            unregisterWindow(window.get());
            window.reset();
        }
    }

    // Focus management
    void setBackground() {
        std::lock_guard<std::mutex> lock(process_mutex);
        if(focused_process.lock().get() == this) {
            focused_process.reset();
        }
        destroyWindow();
    }

private:
    // Static members for process management
    static inline std::vector<std::weak_ptr<OSProcess>> processes_registry;
    static inline std::weak_ptr<OSProcess> focused_process;
    static inline std::mutex process_mutex;

    // Instance members
    Config config;
    TaskHandle_t task_handle = nullptr;
    std::function<void()> execution_code;
    std::unordered_map<uint16_t, std::function<void()>> input_handlers;
    std::unique_ptr<Window> window;

    // Private implementation
    explicit OSProcess(const Config& cfg) : config(cfg) {
        if(config.create_window) {
            createWindow();
        }
        
        // Default safety bindings
        bindInput(0x232B, [this]{ 
            if(window) destroyWindow();
            setBackground();
        });
    }

    static void taskRouter(void* param) {
        auto* proc = static_cast<OSProcess*>(param);
        if(proc->execution_code) {
            proc->execution_code();
        }
        
        // Automatically stop process when execution completes
        proc->stop();
        vTaskDelete(nullptr);
    }

    static void emergencyReturnToMain() {
        // Implementation for system-wide emergency return
    }
};

/* //this used to be the array or whatever
std::vector<std::shared_ptr<OSProcess>> osProcessArray;
std::shared_ptr<OSProcess> FocusedOSProcess;
//GLOBAL REF FOCUSED osproscess: while multiple os proscesses may be on screen at once, only one may accept input at once.                        be sure to have each osproscess accept the  BACK key. (todo: holding back for 10 seconds goes back to main screen reguardless of task or position to avoid stupid crap from happening)
// Example Usage
*/



/*
void createDemoProcess() {
    OSProcess::Config cfg{
        .name = "Demo",
        .create_window = true,
        .priority = 2
    };

    auto proc = OSProcess::create(cfg);
    
    proc->setExecutionCode([]{
        while(true) {
            // Process main loop
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    });

    proc->bindInput(0x2191, []{ 
        Serial.println("Custom up handler!");
    });

    proc->start();
    OSProcess::setFocused(proc);
}


void startBackgroundService() {
    OSProcess::Config cfg{
        .name = "SensorService",
        .stack_size = 8192,
        .priority = tskIDLE_PRIORITY + 2,
        .start_focused = false  // Explicitly background
    };

    auto service = OSProcess::create(cfg);
    
    service->setExecutionCode([]{
        while(true) {
            // Read sensors and process data
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });

    service->start(false);  // Don't focus
}
*/









//tip on asm use:
//define in this order. BASE STRUCTS AND FUNCTION NAMES. THEN extern cpp, starting inline asm. THEN end asm and add implimentations. i think. i figured this out in a day.

struct Vector3 { //todo: fix it. not sure if it works right but whatever i guess
    int16_t x; // Scaled to 2 decimal places
    int16_t y; // Scaled to 2 decimal places
    int16_t z; // Scaled to 2 decimal places

    Vector3(); // Default constructor
    Vector3(float xVal, float yVal, float zVal); // Float constructor
    float magnitude() const; // Magnitude calculation
};


// Implementations
Vector3::Vector3() : x(0), y(0), z(0) {} //output this fuckshit.

Vector3::Vector3(float xVal, float yVal, float zVal) {
    Vector3_ctor(this, xVal, yVal, zVal); // Assembly function
}

float Vector3::magnitude() const {
    return Vector3_magnitude(this); // Call external assembly function
}



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
//warning:each rate limit called increases ram use! 


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






//funny progress bar for that old tech feel
#define PROGRESS_BAR(current, total, width)                    \ 
    do {                                                       \
        Serial.print("[");                                     \
        int progress = (current * width) / total;              \
        for (int i = 0; i < width; i++)                        \
            Serial.print(i < progress ? "#" : "-");            \
        Serial.println("]");                                   \
    } while (0)




//to use
/*
for (int i = 0; i <= 100; i += 10) {
    PROGRESS_BAR(i, 100, 20);
    delay(500);
}

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
#define REPEAT_X_WITH_DELAY(times, delay_ms, on_exec)  \
    do {                                          \
        for (int i = 0; i < (times); i++) {       \
            on_exec;                              \
            delay(delay_ms);                      \
        }                                         \
    } while (0)

//example
/*
REPEAT_X_WITH_DELAY(3, 1000, {
    Serial.println("Delayed Hello!");
});
*/

//uhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh

//NAMING FUCKED UP HERE FIX ITtypedef struct {
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