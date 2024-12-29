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

//chatgpt helped me with this one because god knows i can't do this shit. alone or with about 6 people
//the esp32 DOESN'T HAVE A FUCKING FPU THAT DOES SHIT. LAZY GODDAMN MOTHERFUCKER I'M GOING TO STRANGLE MSELF AND THEM. RAUGH


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

//BECAUSE MUL WAS TOO NORMAL. SO WE HAVE TO SAY MULL? BECAUSE WE'RE QUIRKY. BECAUSE THE DEVELOPERS THINK THEY'RE GOD.
//WELL I'M A GOD TOO. A GODDESS OF FUCKING ASM PAIN. RAAAAAAAAAGUHHHHHH

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







#endif