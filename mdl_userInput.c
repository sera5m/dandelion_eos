#ifndef mdl_userinput_C
#define mdl_userinput_C
//okay, turn opcode into useful instructions via bitfuckery


//switch on the types of inputs. just take a struct type so we can convert that to ints directly. so  wait fuck what do i call this shit. 


//this section enterperets stuff as input.


//section: PHYSICAL buttons
//we're going to have six buttons here. yay. 

//this is going to have to have mercury compat for input routing, so it starts with 16 bit opcodes. 0xF prefix because user input.

//input scheme dat encoding:
//0xF [recognized as user input] x [the hardware type. polymorphic too so it just routes all into the same shit.]


//think a mouse is 0xF4xy, where 4 means mouse, xy is mouse pos
//keyboard is 0xF1ab where 1=keyboard, xy means the key. you get support for 256 keys? amazing. i actually feel good about this!
//xy will be pos for mouses! yay

/*
struct UserInput {
    uint8_t type;  // Hardware type (0xF prefix)
    uint8_t device;  // Device -error throw position: 0, generic keyboard=1 (doesn't matter if it's over bluetooth or wifi, it's a keyboard, just act normal about it),gesture Sense: 3, generic mouse:4
    uint8_t key;  // Key code or position, based on the input
};

//we should use a single opcode to designate input, and just pass it to an included uint8/16 for keys. should be 0xf1??, 0x7???, [8-16 bit address DESIGNATOR OF THE KEY]
//0xf101: USER INPUT, TYPE KEYBOARD, get delay

// Function to decode the mercury opcode into a UserInput struct-replaced with simply a signifier!
UserInput decodeInput(uint16_t opcode) {
    UserInput input;
    input.type = opcode >> 8;  // Extract the 0xF part (high nibble)-this isn't relavent for this part, the oxf just tells us it's some kind of io operation
    input.device = (opcode >> 4) & 0xF;  // Extract device type (e.g., 1 for keyboard, 4 for mouse)
    input.key = opcode & 0xF;  // Extract the key or position (low nibble)
    return input;
}
*/


/*  
wait i have a good idea.
a-z are places [the hex value for 1-27], 0-9 KEYS are 28-38, and so on? we're not gonna consider the idea that numpad keys are different. they're just numbers, for god's sake.
special keys would be annoying but whatevs. 
*/

/*
// Map keyboard keys (a-z, 0-9) to their respective opcodes
uint16_t mapKeyboardInput(char key) {
    if (key >= 'a' && key <= 'z') {
        return 0xF1 + (key - 'a' + 1);  // 'a' -> 0xF1 + 0x01, 'b' -> 0xF1 + 0x02, etc.
    } else if (key >= '0' && key <= '9') {
        return 0xF1 + (key - '0' + 28);  // '0' -> 0xF1 + 0x1C, '1' -> 0xF1 + 0x1D, etc.                yes, representing numbers as 2 digits is weird but that's fine. also my "2" key on my keyboard is broken and i have to hit it extra hard. I don't blame it, it's been 4 years i think you'd just like to know this. haha wasted 30 seconds of ur time 
    }
    // Add logic for special keys like Shift, Ctrl, etc.
    //more logic for function keys
    //if you're not doing this in eng fuck off dude. i'm not doing full ascii in this. if you're german and reading this, fuck your umlauts. 
    //mathematical operators, symbols [/*-+=],[deg,^,|],
    //punctuation !.,;'@#$%&\ 
    return 0;  // Return 0 or some error code if key is not mapped
}
*/
//shift should behave as a key modifier.
// input should just bke [input key] [key condition] [sourceid? or something idk]
// eg [keyboard f key] [condition down] [idk]
//mkay so SHIFT DOWN just denotes "ok capitalize all letters after this untill it's released"









//shuold work like userInput.keyboard(ab) for calling this. but it gets turned into 0xFnab if you're working with this? idk. meow meow meow


//switch on the types of inputs. just take a struct type so we can convert that to ints directly. so  wait fuck what do i call this shit. 




//start defining keyboard input types somewhere else, really. 
#endif