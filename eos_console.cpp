#ifndef eos_console_CPP
#define eos_console_CPP

//this is the console for the user to use- will work over application link, serial monitor in pc, internal function calls

//start defining structs of each of the commands and purposes 



//HOW YOU WILL FUCKIGN DO THIS
//commands will be taken from the user as a string input. a single string and whatever else lol. then they will be COMPRESSED-pseudocompiled as you will, into a propper fucking package for this device. i'll make a buddy program on desktop for this or something, but this will let you make bullshit like quickhacks/arbitrary programs/silly cat simulator whatever fuckign program u wnat
//use this approach, it's more fucking efficient now. WHILE IT IS MY IDEA, MY, BEING THE DESIGNER OF THIS SYMPTOM, I HAD CHATGPT SUMMARIZE MY CAFFENE TWEAKER RANT INTO SOMETHING YOU CAN UNDERSTAND TOMORROW




/*Bytecode Compression for Commands:

    Command Compression: Convert command names (e.g., print, set, run) into fixed-length byte codes (e.g., 3-byte or single-byte). This helps with quick lookups and reduces memory usage.

    Lookup Table: Maintain a lookup table that maps command names to compressed byte codes. This makes it fast to identify and execute commands without needing string comparisons.

    Execution Efficiency: Instead of parsing long strings like "print 10", you convert "print" into a byte code (e.g., 0x01) and then execute it. This avoids comparing strings every time and speeds up command execution.

    Memory Savings: Instead of storing full strings like "print" or "set", store small byte values (0x01, 0x02). This minimizes RAM usage, which is critical for devices like Arduino.

    On Input: When reading a command, immediately convert the string to a byte code using the lookup table, then execute it. This speeds up command parsing and reduces latency.

    Why: This system reduces both memory usage and execution time for command processing. The device can handle more commands faster and more efficiently, even with limited resources.

  storage and execution: when writing program to the hard disk, it will be converted ENTIRELY to bytecodes so it can read and execute em faster. fuck your readability in raw text anyway, you can always just write it out in the desktop link thing i'm going to work on, or whatever's in your phone and so on and so forth.
  readability: it'll be readable when displayed..... so i guess win? win win? good enough for sure, hey!

*/

//formatting: stick to 3 digit hex
// 0Xabc, where a is command type and bc are the comand addresses of type. this should give you a wide set of commands per type, but keep it clear for us developers (me, and the voices in my head :3)



//init modifiers:
//--rush: 
//execute commands while they're still loading from microsd card mem. this is LOW LATENCY but can be fucky if you have wait till's in your code and heavy cpu spikes during it, but whatever lol it's your life bro

//--patient:
//wait for instructions to fully load out of file into mem bnefore exec [larger mem footprint]

//instruction deload modifiers:

//--dementia: device forgets commands as soon as executed: very good for large memory programs/commandline apps [default, why would it not be] KEEPS TEMPORARY COMANDS LIKE "STORE VARIABLE" and waits for tasks to be done! [much like all the other ones]
//adhd: commands deloaded after a cache, at about 4-5 places, with cache being in microsd after ram removal of prior commands 
//elephant: an elephant never forgets (keep all in memory-why are you doing this?)

//use string enumerator swapping. readable but eh on memory, but it's fine since only used twice at the start of each pseudoprogram

//misc exec commands suggested to me
//--async: Trigger background execution, letting you process the script in the background while the device remains responsive.
//--immediate: Ensures commands are executed as soon as they’re read.
//--wait: Pause execution after each command to wait for specific events or conditions before continuing.

//screen options
//--BLIND: DISABLES SCREEN untill turned back on: automatically shows timer icon on screen [impliment later]
//--cataract: screen only updates at low rate, 1/s
//--visual: screen updates at normal rates on core 0
//--gpu: this application needs to update the screen very frequently, change screen to multithread in taskhandling module





/*
types. NOTE THIS IS 16 BIT
range: 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF


POSITION 1:
0XN
N
DEFINED AS, in order
0: hardware manipulation (also used for output)
1
2
3
4
5
6:
7: data manipulation
8: flow controll: 0x8000, 0x8001, 0x
9
a
b
c
d
E: 
f: user interface, input

//this doesn't show every operation, just the main types


*/

#endif