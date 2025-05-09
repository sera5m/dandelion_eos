#ifndef usb_ino
#define usb_ino

//hawwo duckyyy :3

//going to try to use duckyscript-like stuff https://github.com/hak5/usbrubberducky-payloads

struct RubberDuckySequencedDatapoint{ //simple datapoints that work in sequence-best for macros
char key;
uint16_t downtime; //how long to hold the key down, ms
int timefromstart; //ms from start to do this
};
  
enum DuckyScriptCMD{
//put it here, i just can't care rn
};
//todo: load and drop files from internal scr to here-> host pc
//todo: linker tools for host pc->duckymode
//todo: compat with execution of hak5 ducky scripts

//may need to make cmd unifier for this when loading from files in console. commands: <delay 500 ms>, <isnert char x>, conditionals,vars,etc
//callback: on sucess/fail of this, wait, etc

//to fuck with it, i'll just strart the mercury script implimentation. that's the only way i can think to actually exec it, and a universal layer would be lovely


//okay we got the file reader. pretty simple just 
//load the file. fuck it, the whole file. or yk, just parse it into my enums or whatever


#endif


//okay,just chill out and be sure to have things like standard controll and rubberducky parsing or whatever, and also allow pass through