#ifndef MDL_MATH_HELPER_H
#define MDL_MATH_HELPER_H
//this module is made to provide arduino the half dozen math things it doesn't natively support because god fucking help me why woulnd't we have basics like vector 3? 
//noooooo that would bne too fucking hard wouldn't it - sera5m 12/1/2024
//update: it's 3 days later. IT GETS WORSE. WAY WORSE. ASM HELL. I'M IN HELLLLLLLLLLL

//notes on hardware of the esp32:
//fpu: the fpu is absolute garbage. like. faster than me, but still MISERABLY bad for a fpu. (functionally useless proscessor) i'm a person with a soul. and i deserve to not suffer like this. i mean i've had worse life experiences but....
//the cpu or fpu doesn't support division natively. why? that's like. the most normal thing to have. 
//this is wildly different than normal asm. fuck with this at your own peril, AND PERIL YOU WILL GET. 


//tip on asm use:
//define in this order. BASE STRUCTS AND FUNCTION NAMES. THEN extern cpp, starting inline asm. THEN end asm and add implimentations. i think. i figured this out in a day.

struct Vector3 {
    int16_t x; // Scaled to 2 decimal places
    int16_t y; // Scaled to 2 decimal places
    int16_t z; // Scaled to 2 decimal places

    Vector3(); // Default constructor
    Vector3(float xVal, float yVal, float zVal); // Float constructor
    float magnitude() const; // Magnitude calculation
};





//TODO:REMOVE THE ASM

//chatgpt helped me with this one because god knows i can't do this shit. alone or with about 6 people

// Assembly function definitions
extern "C++" { //start inline asm
    void Vector3_ctor(Vector3* self, float xVal, float yVal, float zVal) {
        //take the regular vector as the input and assign to stuff?
        int16_t xInt = static_cast<int16_t>(xVal * 100);
        int16_t yInt = static_cast<int16_t>(yVal * 100);
        int16_t zInt = static_cast<int16_t>(zVal * 100);

        asm volatile( //allocate vars into asm
            "s16i %1, %0, 0\n" // Store x
            "s16i %2, %0, 2\n" // Store y
            "s16i %3, %0, 4\n" // Store z
            :
            : "r"(self), "r"(xInt), "r"(yInt), "r"(zInt));
    }

float Vector3_magnitude(const Vector3* self) {
    int32_t result;  // Use 32-bit integer for intermediate calculations

    asm volatile(
        // Load x, y, z values from the struct (16-bit integers)
        //probably bad

        "l16ui a2, %1, 0\n"      // Load x
        "l16ui a3, %1, 2\n"      // Load y
        "l16ui a4, %1, 4\n"      // Load z


//mul is mull here
        // Calculate x^2 + y^2 + z^2
        "mull a5, a2, a2\n"      // x^2
        "mull a6, a3, a3\n"      // y^2
        "add a5, a5, a6\n"       // x^2 + y^2
        "mull a6, a4, a4\n"      // z^2
        "add a5, a5, a6\n"       // x^2 + y^2 + z^2

        // Integer square root approximation (optimized for larger values)
        "movi a6, 0\n"           // Initialize result = 0
        "movi a7, 0x4000\n"      // Initialize bit = 1 << 14 (16-bit safe)
//TODO: OUTPUT THIS AS INT32. because i'm overflowing i16. given the maximum value of input: 16...... 16*100=1,600 then 1600.... 1600^2= 2,560,000...and the max int16 value is 32767, the max for 32bit is 2.14 billion. so move all this to 32 bit calcs.
// because 32 bit int math is a lot faster than f32. we don't even have to do digit prescision!! yay. BECAUSE NOT ONLY DO I HAVE TO DO CALC AND PRECALC IN ASM. WHAT A GREAT WAY TO SPEND A WENDESDAY MORNING. 
//maybe i can fix this sometime. maybe tomorrow

//start doing square root stuff. i want to kill myself honestly. just yuck
//i don't know how this works. it like.. does a loop of stuff to add stuff together to make the answer ion know

        "1:\n"                   // sqrt_loop
        "blt a7, a5, 2f\n"       // If bit < sum, exit loop
        "add a8, a6, a7\n"       // temp = result + bit
        "bge a5, a8, 3f\n"       // If sum >= temp, update result
        "j 4f\n"                 // Otherwise, skip update

        "3:\n"                   // sqrt_update
        "sub a5, a5, a8\n"       // sum -= temp
        "or a6, a6, a7\n"        // result |= bit

        "4:\n"                   // sqrt_skip
        "srli a7, a7, 2\n"       // bit >>= 2
        "j 1b\n"                 // Repeat loop

        "2:\n"                   // sqrt_done
        "mov %0, a6\n"           // Store final result in output

        : "=r"(result)           // Output
        : "r"(self)              // Input
        : "a2", "a3", "a4", "a5", "a6", "a7", "a8" // Clobbered registers
    );

    // Scale back the result by dividing by 100 to account for the initial scaling
    return static_cast<float>(result) / 100.0f;
} //just realized i implimented this wrong, gotta out f32. because... i don't know. i just don't. bits add i guess?

//meow meow meow meow meow meow meow meow meow meow meow meow meow meow meow meow meowmeowmeowmeowmeoowwwwmeowmeowmeowmraaameowmeowmeowwwwmraaaammmmmmrmmrmrmrmmrmrmmamamaaaa

}//ends the inline asm

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

//to use: deleteRateLimit(myRateLimiter);

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
int myTimer = createTimer(5000);  // 5-second timer

void loop() {
    LOOP_WHILE_TIMER(myTimer, 500, {
        Serial.println("Tick!");
    });

    // To pause:
    // PAUSE_TIMER(myTimer);

    // To resume:
    // RESUME_TIMER(myTimer);

    // To reset:
    // RESET_TIMER(myTimer);
}

*/



#endif