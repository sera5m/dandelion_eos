#ifndef watch_settings_H
#define watch_settings_H

//default colors for face
//tpallt means theme pallette
struct ui_color_pallette {
    uint16_t primary;
    uint16_t secondary;
    uint16_t tertiary;  
    uint16_t highlight;
    uint16_t background;
};

//default theme, plain
ui_color_pallette UITHM_mint{
    0x07ff, //teal
    0x77f9, //i can't find a good green
    0xe4ff,//lavender
    0xd7fd,//very light green highlight
    0x29e6//background
};
// Other themes
ui_color_pallette UITHM_hacker = {
    0x07E0, // green
    0x0000, // black
    0x4208, // dark gray
    0x07F0, // neon green
    0x0000  // black
};

ui_color_pallette UITHM_specOps = {
    0x8803, // deep red
    0x0001, // near black purple
    0x300F, // deep indigo
    0xF805, // ops pink
    0x0000  // black
};

ui_color_pallette UITHM_terminal = {
    0x07E0, // terminal green
    0x0000, // black
    0x4208, // old screen gray
    0xC618, // light gray
    0x0000  // black
};
ui_color_pallette UITHM_userCustom = {
    0x07EF, 
    0x0000, 
    0x4208, 
    0xFCCF, 
    0x0000  
};



extern uint16_t tcol_primary;
extern uint16_t tcol_secondary;
extern uint16_t tcol_tertiary;
extern uint16_t tcol_highlight;
extern uint16_t tcol_background;

enum list_Themes{mint,hacker,specOps,terminal,userCustom};

extern list_Themes Current_Theme;

void SetDeviceTheme(list_Themes theme) {
    ui_color_pallette selectedTheme;

    switch (theme) {
        case mint:
            selectedTheme = UITHM_mint;
            break;
        case hacker:
            selectedTheme = UITHM_hacker;
            break;
        case specOps:
            selectedTheme = UITHM_specOps;
            break;
        case terminal:
            selectedTheme = UITHM_terminal;
            break;
        case userCustom:
            // Set your custom theme loading logic here
            return;
    }

    tcol_primary   = selectedTheme.primary;
    tcol_secondary = selectedTheme.secondary;
    tcol_tertiary  = selectedTheme.tertiary;
    tcol_highlight = selectedTheme.highlight;
    tcol_background= selectedTheme.background;
    //todo in the future have this just refresh all the things by calling to window manager
}




#endif

