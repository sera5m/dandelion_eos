#include "globals.h"
#include <Arduino.h>
#include "Micro2D_A.h"
uint16_t tcol_primary = 0x0EFF;
uint16_t tcol_secondary = 0x88FB;
uint16_t tcol_tertiary = 0xE4FF;
uint16_t tcol_highlight = 0xDBBF;
uint16_t tcol_background = 0x2000;



std::shared_ptr<Window> Win_GeneralPurpose; 
uint16_t WatchScreenUpdateInterval=500;
 //size defined in the h file
char watchscreen_buf[WATCHSCREEN_BUF_SIZE];









extern int AVG_HR;


int16_t temp_c=69;


ui_color_palette UITHM_mint={
    0x07ff, //teal
    0x77f9, //i can't find a good green
    0xe4ff,//lavender
    0xd7fd,//very light green highlight
    0x29e6//background
    
};
// Other themes
ui_color_palette UITHM_hacker = {
    0x07E0, // green
    0x0000, // black
    0x4208, // dark gray
    0x07F0, // neon green
    0x0000  // black
};

ui_color_palette UITHM_specOps = {
    0x8803, // deep red
    0x0001, // near black purple
    0x300F, // deep indigo
    0xF805, // ops pink
    0x0000  // black
};

ui_color_palette UITHM_terminal = {
    0x07E0, // terminal green
    0x0000, // black
    0x4208, // old screen gray
    0xC618, // light gray
    0x0000  // black
};
ui_color_palette UITHM_userCustom = {
    0x07EF, 
    0x0000, 
    0x4208, 
    0xFCCF, 
    0x0000  
};