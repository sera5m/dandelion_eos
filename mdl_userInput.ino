#ifndef mdl_userinput_ino
#define mdl_userinput_ino


#include <PCF8574.h>



//todo: if the back button is held for ten seconds, exit this and go back

/*
//this doesn't really do anything yet

// Function to handle button events
void OnButtonPush(bool isDown, hardwareButtons button) {
  if (isDown) {

    // Button pressed
    switch (button) {
      case Up:    Serial.println("Up button pressed");    break;
      case Down:  Serial.println("Down button pressed");  break;
      case Left:  Serial.println("Left button pressed");  break;
      case Right: Serial.println("Right button pressed"); break;
      case Enter: Serial.println("Enter button pressed"); break;
      case Back:  Serial.println("Back button pressed");  break;
    }
  } else {
    // Button released
    switch (button) {
      case Up:    Serial.println("Up button released");    break;
      case Down:  Serial.println("Down button released");  break;
      case Left:  Serial.println("Left button released");  break;
      case Right: Serial.println("Right button released"); break;
      case Enter: Serial.println("Enter button released"); break;
      case Back:  Serial.println("Back button released");  break;
    }
  }
}
//todo:set these to take unicode and dump that into 


*/






//okay, turn opcode into useful instructions via bitfuckery





//please ignore the prior user input thingy. keyboard inputs should be ascii, but set inline with stuff [opcode to say keycode] [key value] [terminate keycode] [other instructions]









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



//shift should behave as a key modifier.
// input should just bke [input key] [key condition] [sourceid? or something idk]
// eg [keyboard f key] [condition down] [idk]
//mkay so SHIFT DOWN just denotes "ok capitalize all letters after this untill it's released"









//shuold work like userInput.keyboard(ab) for calling this. but it gets turned into 0xFnab if you're working with this? idk. meow meow meow


//switch on the types of inputs. just take a struct type so we can convert that to ints directly. so  wait fuck what do i call this shit. 




//start defining keyboard input types somewhere else, really. 

#endif