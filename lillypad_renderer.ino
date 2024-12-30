// LILLYPAD_RENDERER.h

#ifndef LILLYPAD_RENDERER_H
#define LILLYPAD_RENDERER_H

//was gonna name this orchid renderer but some guy made a render engine already named that and it was cool tbh



//this is the window handler for dandelion e-os. if you want low level stuff about the screen see module_math_render_base. that module handles direct controll of the screen
//this module handles window creation and draw calls. version 4, now with performance enhancements. :3


//todo: automatically store a registry of all known windows
//need to remove it from the list and from collection because it doesn't exist
//we'll also have to use a destructor to clean this object up!



// Window structure to hold window properties
struct WindowCfg {
    int x, y;          // Position
    int width, height; // Size
    bool auto_alignment=0,wrap=1; // Align text centrally or not,wrap text
    int textsize=1; //default text size inside this window
    uint16_t borderColor, bgColor, text_color; // Colors -add defaults
}; 



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



    // Constructor
    Window(const std::string& windowName, const WindowCfg& cfg, const std::string& initialContent = "")
        : name(windowName), config(cfg), content(initialContent) {}


    // Destructor: Clean up canvases
    ~Window() {
        for (Canvas* canvas : canvases) {
            delete canvas;
        }
    }


//canvas
void addCanvas(const CanvasCfg& cfg) {
    if (cfg.parentWindow != this) {
        Serial.println("Error: Canvas parent does not match this window.");
        return;
    }
    canvases.push_back(new Canvas(cfg));
}



    // Draws the window with its content
    void draw() {
        // Clear the window interior
        tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor);

        // Draw the window outline
        tft.drawRect(config.x, config.y, config.width, config.height, config.borderColor);

        // Set text properties
        tft.setTextColor(config.text_color);
        tft.setTextSize(config.textsize); //we will need to be able to draw multiple strings to this. I'd really like dynamic size set and multi string support

        // Render content as word-wrapped text
        drawText(content.c_str());
    }




//clear the window
void clear() {
    tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor);
}


    // Updates window content and redraws
    void updateContent(const std::string& newContent) {
        content = newContent;
        draw();
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
};

//example logic for the window registry. this would have to be some kind of dynamic array? idk
/*
std::vector<Window*> windowRegistry;

void registerWindow(Window* win) {
    windowRegistry.push_back(win);
}

void unregisterWindow(Window* win) {
    windowRegistry.erase(std::remove(windowRegistry.begin(), windowRegistry.end(), win), windowRegistry.end());
}

//need to extend this logic
*/




//*lego building noise* i made a lot of this on 12/30/2024. call that a buzzer beater


//the logic for draw canvases

// kina like the window
struct CanvasCfg {
    int x, y;          // Position RELATIVE TO PARENT WINDOW
    int width, height; // Size
    Window* parentWindow; //get a ref to the parent window. canvases may not have multiple parents
    uint16_t borderColor, bgColor, text_color; // Colors -add defaults
}; 


//start a new canvas thingy
class canvas {

int CanvasUpdateMS=500; //ms of update for canvas
CanvasCfg config; //load the canvas config


public:{


    // Constructor: Initialize canvas with position, size, color, and parent window
    Canvas(int posX, int posY, int w, int h, uint16_t color, Window* parent)
        : x(posX), y(posY), width(w), height(h), bgColor(color), parentWindow(parent) {}

    


//draw canvas content here, kina like the win but with new stuff
     void draw() {
        if (!parentWindow) {
            Serial.println("Error: Canvas has no parent window.");
            return;
        }

        // Get the canvas's absolute position within the parent window
        int canvasX = parentWindow->config.x + x;
        int canvasY = parentWindow->config.y + y;

        // Draw canvas background
        tft.fillRect(canvasX, canvasY, width, height, bgColor);

        // Draw canvas border
        tft.drawRect(canvasX, canvasY, width, height, borderColor);
    }
//this is just base setup logic, canvases don't seem to have elements yet? 
//elements should be fixed size, or even things like ug8lib drawings? or where we can put images? idunno

    // Draw a rectangle inside the canvas
    void drawRect(int posX, int posY, int w, int h, uint16_t color) {
        int canvasX = parentWindow->config.x + x;
        int canvasY = parentWindow->config.y + y;
        tft.fillRect(canvasX + posX, canvasY + posY, w, h, color);
    }

  }




void clear() {
    tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor); //my ass just copied this from the window idk lol aha this better work here
}



}

private:{


}



} //end canvas work thing




//some defaults idk


//variables used in this function to drawscreen
// Global variables for time access
extern int currentHour;
extern int currentMinute;
extern int currentSecond;
extern float temperature;
extern int AVG_HR;

//define a little window group we can set for the lock screen
//quick,dirty,effective
#define defaultWinGroup_lockscreen \
// Predefine windows for the lock screen
Window timeWindow("Time", WindowCfg{14, 34, 100, 40, false, true, 2, 0xFFFF, 0x0000, 0x07FF}, "00:00");
Window tempWindow("Temperature", WindowCfg{10, 0, 100, 30, false, true, 1, 0xFFFF, 0x0000, 0x558F}, "23C");
Window heartRateWindow("Heart Rate", WindowCfg{100, 120, 50, 30, false, true, 1, 0xFFFF, 0x0000, 0xB000}, "72");

void updateLockScreen() {
    heartRateWindow.updateContent(std::to_string(AVG_HR)); // Update heart rate window
    tempWindow.updateContent(std::to_string(temperature)); // Update temperature window

    // Format time as "hh:mm:ss" byconvert time into hhmmss
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
