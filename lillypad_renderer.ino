// LILLYPAD_RENDERER.h

#ifndef LILLYPAD_RENDERER_H
#define LILLYPAD_RENDERER_H

//was gonna name this orchid renderer but some guy made a render engine already named that and it was cool tbh



//this is the window handler for dandelion e-os. if you want low level stuff about the screen see module_math_render_base. that module handles direct controll of the screen
//this module handles window creation and draw calls. version 4, now with performance enhancements. :3




//forward declare all possible dependencies/child of window before we use them

class Canvas;  // forward declaration 
class Window; //do i have to define the window class?
//include the struct dependencies of children




//declare structs and stuff of the elements of this stupid thing.

// Window structure to hold window properties
struct WindowCfg {
    int x = 0, y = 0, width = 64, height = 64;
    bool auto_alignment=0,wrap=1; // Align text centrally or not,wrap text
    int textsize=1; //default text size inside this window
    bool borderless=false;
    uint16_t borderColor, bgColor, text_color; // Colors -add defaults
}; 




struct CanvasCfg {
    int x = 0, y = 0, width = 32, height = 32;
    bool borderless=true;
     uint16_t bgColor = 0x0000, borderColor = 0xFFFF;
     Window* parentWindow;
};
//end forward dependencies


//****************************************************************************************************************************************

//canvas draw logic
//canvases are pannels that you can draw various things in, and they'll keep it from spilling off. great for doing stuff like windows

//******************************************************************************************************************************************************


// kina like the window struct setup, but not identical because it takes a parent


// Canvas class definition
// Canvas class definition
class Canvas {
public:
    int x, y, width, height;
    uint16_t bgColor, borderColor;
    

    Canvas(const CanvasCfg& cfg, Window* parent): x(cfg.x), y(cfg.y), width(cfg.width), height(cfg.height),bgColor(cfg.bgColor), borderColor(cfg.borderColor), parentWindow(parent) {}

    void clear() {
        // Use the config directly
        tft.fillRect(x, y, width, height, bgColor);
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    CanvasCfg config;
    Window* parentWindow; //ref to parent window. [should i add reparenting??]

};



//****************************************************************************************************************************************


//window draw logic

//windows are pannels that you can have text and attatch to a proscess to directly write to or modify. all children like widgets or canvas pannels need to be attatched to windows.
//windows are what you call update on, so they update sub elements

//******************************************************************************************************************************************************

//have the windows treated as objects this time instead of just functions. makes referencing much more reliable lol

class Window {
public:

    std::string name;  // Window's name
    WindowCfg config; //take config
    std::string content; //take string off this

    //std vector is kina like dynamic arrays
    std::vector<Canvas*> canvases;  // List of canvases attached to this window


    //other internal variables for the windows themselves
    int WinUpdateMS=500; //this window should update every n miliseconds. this is the variable and it can be dynamically set with a reference to this window by attatched proscesses.
    bool dirty = false;  // does window need redraw



    // Constructor
    Window(const std::string& windowName, const WindowCfg& cfg, const std::string& initialContent = "")
        : name(windowName), config(cfg), content(initialContent) {}


    // Destructor: Clean up canvases
    ~Window() {
        for (Canvas* canvas : canvases) {
            delete canvas;
        }
    }



  //add a canvas to this window.
void addCanvas(const CanvasCfg& cfg) {
    if (cfg.parentWindow != this) {
        Serial.println("Error: Canvas parent does not match this window.");
        return; 
    }
    Canvas* newCanvas = new Canvas(cfg, this);  // create new canvas and assign it to this window
    //keyword new tells the program i'm creating a new object and to give it some memory. 
    canvases.push_back(newCanvas);  // add the canvas to the std vector (arr)
}



    // Draws the window with its content
    void draw() {
      if (dirty) {
        // Clear the window interior
        tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor);

        // Draw the window outline
        tft.drawRect(config.x, config.y, config.width, config.height, config.borderColor);

        // Set text properties
        tft.setTextColor(config.text_color);
        tft.setTextSize(config.textsize); //we will need to be able to draw multiple strings to this. I'd really like dynamic size set and multi string support

        // Render content as word-wrapped text
        drawText(content.c_str());
                dirty = false;
                }
    }




//clear the window
void clear() {
    tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor);
}


void updateContent(const std::string& newContent) {
    if (content != newContent) { //check to see if it matches. if it does, don't update it
        content = newContent;
        dirty = true; //k it's tirty so go draw that stuff!
        draw(); //draw it now
    }
}

private:




    void drawText(const char* text) {
        int charWidth = 6;  // Character width
        int charHeight = 8; // Character height
        String strText = String(text);
        int cursorX = config.x + 2;  // Padding
        int cursorY = config.y + 2;

        int wordStart = 0;
        for (int i = 0; i <= strText.length(); i++) {
            if (strText[i] == ' ' || strText[i] == '\0') {
                String word = strText.substring(wordStart, i);
                int wordWidth = word.length() * charWidth;

                // Check if the word fits in the current line
                if (cursorX + wordWidth > config.x + config.width - 2) {
                    cursorX = config.x + 2;
                    cursorY += charHeight;
                    if (cursorY + charHeight > config.y + config.height - 2) {
                        break;  // Overflow
                    }
                }

                tft.setCursor(cursorX, cursorY);
                tft.print(word);
                cursorX += wordWidth + charWidth;
                wordStart = i + 1;
            }
        }
    }



std::vector<std::unique_ptr<Window>> windowRegistry;

void registerWindow(std::unique_ptr<Window> win) {
    windowRegistry.push_back(std::move(win));
}

void unregisterWindow(Window* win) {
    windowRegistry.erase(
        std::remove_if(
            windowRegistry.begin(),
            windowRegistry.end(),
            [&](const std::unique_ptr<Window>& w) { return w.get() == win; }),
        windowRegistry.end());
}

}; //the end of the window class














///***************************************************************************************************************************************************************************************************************************************************************
//operating system defaults for various types of screen setups
//default groupings are groups of default windows to put on the screen in some conditions, saving you time from having to manually add one of each window to the screen [lock screen,app screen, etc]
///***************************************************************************************************************************************************************************************************************************************************************


//

//variables used in this function to drawscreen
// Global variables for time access
extern int currentHour;
extern int currentMinute;
extern int currentSecond;
extern float temperature;
extern int AVG_HR;

// Define a little window group we can set for the lock screen
// Quick, dirty, effective
#define defaultWinGroup_lockscreen \
//as of jan 2, 2025, it should be  x,y,w,h,auto align?,wrap?,textsize,borderkess?, border color,bgcolor,txt color, new text to write
Window timeWindow("Time", WindowCfg{14, 34, 100, 40, false, true, 2,false, 0xFFFF, 0x0000, 0x07FF}); \
Window tempWindow("Temperature", WindowCfg{10, 0, 100, 30, false, true, 1, true, 0xFFFF, 0x0000, 0x558F}); \
Window heartRateWindow("Heart Rate", WindowCfg{100, 120, 50, 30, false, true, 1,false, 0xFFFF, 0x0000, 0xB000}); \

// TODO: With the clock module overhaul, this section will need a lot of fixes.

void updateLockScreen() {
    heartRateWindow.updateContent(std::to_string(AVG_HR)); // Update heart rate window
    tempWindow.updateContent(std::to_string(IMU.getTemp())); // Update temperature window (updated to use the sensor inside the imu for reduced hardare cost.)

    // Format time as "hh:mm:ss" by converting time into hh:mm:ss
    std::string timeString = 
        (currentHour < 10 ? "0" : "") + std::to_string(currentHour) + ":" +  // Ensure 2 digits for hour
        (currentMinute < 10 ? "0" : "") + std::to_string(currentMinute) + ":" +  // Ensure 2 digits for minute
        (currentSecond < 10 ? "0" : "") + std::to_string(currentSecond);  // Ensure 2 digits for second
    timeWindow.updateContent(timeString); // Update time window
}

/*
void updateLockscreenTime(int hour, int minute, int second) {
    std::string timeStr = (hour < 10 ? "0" : "") + std::to_string(hour) + ":" + (minute < 10 ? "0" : "") + std::to_string(minute);
    timeWindow.updateContent(timeStr); // Update time window

    std::string secondStr = (second < 10 ? "0" : "") + std::to_string(second);
    heartRateWindow.updateContent(secondStr); // Update heart rate window (using seconds for simplicity)
}

void updateLockscreenTemperature(float temperature) {
    tempWindow.updateContent(std::to_string((int)temperature) + "C"); // Update temperature window
}
*/

#endif
