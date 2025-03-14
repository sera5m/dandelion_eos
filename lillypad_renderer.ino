// LILLYPAD_RENDERER.h

#ifndef LILLYPAD_RENDERER_H
#define LILLYPAD_RENDERER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <memory>

//todo:

//little alert icons
//jpeg+png+bmp

//at attatches to canvas. vect drawings just use a few simple lines and cubes. we save them on the setup of the drawing object, with it storing the commands on start? idk how to do this outside of vague ideas.
//crude animations [involves placing objects on the canvas, with objects being an image or "generated vector drawing"] 

//optimize update cycle

//wrap stuff around like text. just read uglib and use that stuff i guess. we can take heavy inspiration from the code.
//scroll boxes please
//dirty flag should be improved. needs tohave autoatic draw calls


//was gonna name this orchid renderer but some guy made a render engine already named that and it was cool tbh



//this is the window handler for dandelionn. if you want low level stuff about the screen see module_math_render_base. that module handles direct controll of the screen
//this module handles window creation and draw calls. version 4, now with performance enhancements. :3

#include "module_math_render_base.ino" //i hate this verbose name
extern Adafruit_SSD1351 tft;  /* fucking shit needs to be imported. why is this not treated as a global object from the fucking screen setup*/


//forward declare all possible dependencies/child of window before we use them

class Canvas;  // forward declaration 
class Window; //do i have to define the window class?
class WindowManager; //only make one of these on os start
//include the struct dependencies of children

//userspace graphics config import do not delete this or the whole system will fuck up
extern bool AreGraphicsEnabled = true;//userconfig
extern bool isWindowHandlerAlive = false;//do not touch this in user code ever, for the window manager code only. "hey guys what if we could break the entire goddamn window manager on a whim!!!" "get the fuck outta my office"


//declare structs and stuff of the elements of this stupid thing.

// Window structure to hold window properties
struct WindowCfg {
    int x = 0, y = 0, width = 64, height = 64;
    bool auto_alignment=0,wrap=1; // Align text centrally or not,wrap text
    int textsize=1; //default text size inside this window
    bool borderless=false;
    uint16_t borderColor, bgColor, text_color; // Colors -add defaults
    uint16_t UpdateRateMs=500;//update every how many ms?
}; 




struct CanvasCfg {
    int x = 0, y = 0, width = 32, height = 32;
    bool borderless=true;
     uint16_t bgColor = 0x0000, borderColor = 0xFFFF;
      Window* parentWindow = nullptr;
};



struct PixelDat{ //wanna draw a shitton of individual pixels with no care for efficiency? (i assume used for graphing)
int posX;
int posY;
uint16_t color;
int layer;
};


//end forward dependencies

//todo: ALL THE UPDATE RATE TICKS/WHATEVER ARE THE WRONG VAR NAMES! THEY DO NOT AGREE! MAKE THEM MATCH! this will mean the damn things won't do shit unless i make it work

//****************************************************************************************************************************************

//canvas draw logic
//canvases are pannels that you can draw various things in, and they'll keep it from spilling off. great for doing stuff like windows

//******************************************************************************************************************************************************


// A DrawableElement represents any element (text or shape) drawn on the canvas.
struct DrawableElement {
    uint layer;  // Lower layer drawn first, higher layers drawn on top
    std::function<void()> drawFunc; // Lambda that draws this element
};

// Canvas class definition

class Canvas {
public:
    // Position and size of the canvas on screen (relative to parent window if needed)
    int x, y, width, height;
    uint16_t bgColor, borderColor;
    
    // Flag to indicate if canvas needs updating.
    bool canvasDirty = true;
    bool borderless;  //now takes borderless from struct

    // Container for drawable elements on this canvas.
    std::vector<DrawableElement> drawElements; //replace with function ptr in future
    
    // Reference to parent window (if any)
    Window* parentWindow;
    
    // Constructor: initializes from a CanvasCfg structure and assigns parent pointer.
    Canvas(const CanvasCfg& cfg, Window* parent)
        : x(cfg.x), y(cfg.y), width(cfg.width), height(cfg.height),
          bgColor(cfg.bgColor), borderColor(cfg.borderColor),
          parentWindow(parent) {}
    
    // Clear the canvas area
    void clear() {
        // Fill the canvas area with the background color.
        // Note: if you need clipping, ensure tft supports it or implement it yourself.
        tft.fillRect(x, y, width, height, bgColor);
        canvasDirty = true;
    }
    
    // Adds a text line to the canvas.
    // (No wrapping—if the text extends beyond the canvas, it is simply clipped.)
    void AddTextLine(int posX, int posY, const String &text, uint16_t textColor, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=, text]() { //forces string copy so it's still available for lambda after this goes outta scope. call this function with your text to draw it
            tft.setTextColor(textColor);
            // Set the cursor relative to canvas origin.
            tft.setCursor(x + posX, y + posY);
            tft.print(text);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }




//draw shapes **********************************************************************************
//********************these are shapes the user should be calling to draw in canvas,each one should be set to it's own layer-non automatically set as of now, cope. plus you get more controll**********************************************

 void AddLine(int posX0, int posY0, int posX1, int posY1, uint16_t color, int layer = 0){
 DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw

        // Check if it's a vertical line
        if (posX0 == posX1) {
            tft.drawFastVLine(x + posX0, y + std::min(posY0, posY1), std::abs(posY1 - posY0), color); //x,y,len,color-should be void drawFastVLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color); make sure params agree with bit sizes
        }
        // Check if it's a horizontal line
        else if (posY0 == posY1) {
            tft.drawFastHLine(x + std::min(posX0, posX1), y + posY0, std::abs(posX1 - posX0), color); //void drawFastVLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color); params need to agree w bit size
        }
        // Otherwise, draw a normal angled line
        else {
            tft.drawLine(x + posX0, y + posY0, x + posX1, y + posY1, color);
        }
    };
        drawElements.push_back(element);
        canvasDirty = true;
}
  //adafruit has optimized line drawing and normal line drawing, for an angular one it's drawLine,
  // for a perfectly vertical or horizontal line it's  drawFastVLine or drawFastHLine. fortunately they take simular args, so here i've switched between them




    void AddPixel(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.drawPixel(x + posX, y + posY, color); //args, posx posy, color
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }




//draw multiple pixels on multiple layers
  void AddPixels(const std::vector<PixelDat>& pixels) { //takes an array of pixel pos
    DrawableElement element;
    element.layer = pixels.empty() ? 0 : pixels[0].layer; // Default to layer 0 if empty

    element.drawFunc = [&]() { // Lambda to draw all pixels
        for (const auto& pixel : pixels) {
            tft.drawPixel(x + pixel.posX, y + pixel.posY, pixel.color);
        }
    };

    drawElements.push_back(element);
    canvasDirty = true;
  }


 // drawing a rectangle.

//draw filled rect
    void AddFRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.fillRect(x + posX, y + posY, w, h, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
//draw a rect, not filled.
    void AddRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.drawRect(x + posX, y + posY, w, h, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }



 //roundeded rectangles



//DRAW ROUNDED rect,filled
    void AddRFRect(int posX, int posY, int w, int h,uint16_t r, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.fillRoundRect(x + posX, y + posY, w, h, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
//draw a rounded rect, not filled.
    void AddRRect(int posX, int posY, int w, int h,uint16_t r, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.drawRoundRect(x + posX, y + posY, w, h, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }




//tri,hollow
    void AddTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, int layer = 0) { //args take one position per edge
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
         tft.drawTriangle(x0+x,y0+y,x1+x,y1+y,x2+x,y2+y, color); //positions of parent adn position of the 3 points taken 
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }


//filled triangle
    void AddFTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, int layer = 0) { //args take one position per edge
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
         tft.fillTriangle(x0+x,y0+y,x1+x,y1+y,x2+x,y2+y, color); //positions of parent adn position of the 3 points taken 
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }


//circles
//cirlces are drawn with radius r around centerpoint xy pos with color

//filled circle
    void AddFCircle(int posX, int posY, int r, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.fillCircle(x + posX, y + posY, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
//draw a ciurcle, not filled.
    void AddCircle(int posX, int posY, int r, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [&]() { //call this to draw
        tft.drawCircle(x + posX, y + posY, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }




//back to main draw logic for canvas here



    // Draws the canvas: first draws the canvas background (and border if not borderless),
    // then sorts and draws the drawable elements by layer.
    void Draw() {
          unsigned long startTime = millis(); //start time for the frame
        // Clear canvas area
        tft.fillRect(x, y, width, height, bgColor);
        if (!borderless) {
    tft.drawRect(x, y, width, height, borderColor);
       }
        
        // Sort drawable elements by layer (lowest first)
        std::sort(drawElements.begin(), drawElements.end(), [](const DrawableElement &a, const DrawableElement &b) {
            return a.layer < b.layer;
        });
        
        // Draw each element
        for (auto &elem : drawElements) {
            elem.drawFunc();
        }
    lastFrameTime = millis() - startTime; //end of frame
    canvasDirty = false; //now it's clean
    }
    
    /*
    // A simple function to determine borderless state.
    bool IsBorderless() const { //todo! fix: borderless is a declared input param. why do we need this? just do if else for borderless in the canvas

        // If borderless is desired, return true.
        // (Alternatively, you can store a borderless flag in this class.)
        return false; // For now, assume border should be drawn.
    }
    */



    // Optionally update canvas only if dirty
    void update() {
    unsigned long now = millis();
    if (canvasDirty || (now - lastUpdateTime >= TryUpdateInterval)) {
        Draw();
        lastUpdateTime = now;
    }
}


 private:

unsigned long lastUpdateTime = 0;   // in ms
unsigned int TryUpdateInterval = 100;    // default update rate (ms) for canvas
unsigned int lastFrameTime = 0;       // duration of last canvas draw



};


//****************************************************************************************************************************************


//window draw logic

//windows are pannels that you can have text and attatch to a proscess to directly write to or modify. all children like widgets or canvas pannels need to be attatched to windows.
//windows are what you call update on, so they update sub elements

//******************************************************************************************************************************************************
//remember, constructor uses 
/*struct WindowCfg 
    int x = 0, y = 0, width = 64, height = 64;
    bool auto_alignment=0,wrap=1; // Align text centrally or not,wrap text
    int textsize=1; //default text size inside this window
    bool borderless=false;
    uint16_t borderColor, bgColor, text_color; // Colors -
    uint16_t UpdateRateMs=500;//update every how ms? todo: this is unclear, and windows should only update while dirty anyway. but an occasional refresh wouldn't hurt for just in case, so i've decided to make this either rate limit the update or enforce it every interval. not sure. 
 *///DO NOT UNCOMMENT, THIS IS HERE TO SHOW YOU

class Window {
public:
    std::string name;       // Window's name
    WindowCfg config;       // Window configuration
    std::string content;    // Full text content (may be longer than visible area)

    // List of canvases attached to this window
    std::vector<Canvas*> canvases;

    int UpdateTickMs = UpdateTimeMs;  // update interval in ms-take from config now! yay i think
    bool dirty = false;     // uased as redraw flag?

    // NEW: Scrolling members:
    int scrollOffset = 0;                // Vertical scroll offset in pixels
    std::vector<std::string> wrappedLines;    // Wrapped text lines for rendering

    // Constructor
    Window(const std::string& windowName, const WindowCfg& cfg, const std::string& initialContent = ""): name(windowName), config(cfg), content(initialContent) {}

    // Destructor: Clean up canvases
    ~Window() {for (Canvas* canvas : canvases) {delete canvas;}canvases.clear();} //clear all the canvas instances on each window and clear


void setUpdateRate(int newRate) {
    UpdateTickMs = newRate;
    WindowManager::getInstance().notifyUpdateRateChange(this); //change the variable in the update rate
}


    // Add a canvas to this window
    void addCanvas(const CanvasCfg& cfg) {

        if (cfg.parentWindow != this) {
        Serial.print("Error: Canvas parent mismatch in window: "); Serial.println(name.c_str());
        //reject request, it's on the wrong line
            return;
        }
        //safe,create new canvas+pushback
        Canvas* newCanvas = new Canvas(cfg, this); 
        canvases.push_back(newCanvas);
    }

    // Update the window's content and mark as dirty
    void updateContent(const std::string& newContent) {
        if (content != newContent) {
            content = newContent;
            dirty = true;
            draw(); // immediate redraw
        }
    }

    // Scroll functions: call these from your input handling code
    void scrollUp(int pixels) {
        scrollOffset -= pixels;
        if (scrollOffset < 0) scrollOffset = 0;
        dirty = true;
        // Mark all child canvases as dirty
        for (Canvas* c : canvases) {
            c->canvasDirty = true;
        }
    }
    void scrollDown(int pixels) {
        int totalTextHeight = wrappedLines.size() * (8 * config.textsize);
        int maxOffset = (totalTextHeight > config.height - 4) ? totalTextHeight - (config.height - 4) : 0;
        scrollOffset += pixels;
        if (scrollOffset > maxOffset) scrollOffset = maxOffset;
        dirty = true;
        for (Canvas* c : canvases) {
            c->canvasDirty = true;
        }
    }

    // Draws the window and its text content, then updates child canvases if they're visible.
    void draw() {
          unsigned long startTime = millis();//framerate counter, time counter start
        // Clear the window area
        tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor); //fill the window with the background color
        if (!config.borderless) //if window not borderless
            tft.drawRect(config.x, config.y, config.width, config.height, config.borderColor); //draw the rectangle outline

        // Set text properties
        tft.setTextColor(config.text_color);
        tft.setTextSize(config.textsize);

        // Update wrapped lines from full content
        updateWrappedLines();

        // Calculate text metrics and draw only visible lines
        int charHeight = 8 * config.textsize;  // approximate text height per line
        int visibleLines = (config.height - 4) / charHeight;  // available lines in window
        int startLine = scrollOffset / charHeight;
        int y = config.y + 2 - (scrollOffset % charHeight);

        for (int i = startLine; i < wrappedLines.size() && i < startLine + visibleLines; i++) {
            tft.setCursor(config.x + 2, y);
            tft.print(wrappedLines[i]);
            y += charHeight;
        }
            lastFrameTime = millis() - startTime;
    dirty = false;

        // Update/draw each canvas only if it's at least partially within the window.
        for (Canvas* c : canvases) {
            // Calculate the canvas's absolute position relative to the window.
            int canvasAbsX = config.x + c->x;
            int canvasAbsY = config.y + c->y;
            // Check if the canvas is completely off-screen relative to the window.
            if ((canvasAbsX + c->width) < config.x || canvasAbsX > (config.x + config.width) ||(canvasAbsY + c->height) < config.y || canvasAbsY > (config.y + config.height)) {
                //(if x> possible and y>possible)
                continue;  // Skip drawing this canvas if it's entirely off-screen.
            }
            c->update();  // Update/draw the canvas.
        }
    }

private:

unsigned long lastUpdateTime = 0;  // in ms

unsigned int lastFrameTime = 0;    // duration of last draw


//todo: line centering if i haven't done it already

    // Wrap the full content into lines that fit the window width.
    // This simplistic approach uses a fixed-width estimation.
    void updateWrappedLines() {  //note: i tried changing all strings to std::string, may not have been done right

        wrappedLines.clear(); //todo: why is this here? clear existing lines?

    
        int charWidth = 6 * config.textsize; //take text size and default width to see at any size
        int maxCharsPerLine = (config.width - 4) / charWidth;
        
    std::istringstream textStream(content);
    std::string word, currentLine;

    while (textStream >> word) {

        if (currentLine.length() + word.length() + 1 > maxCharsPerLine) {
            wrappedLines.push_back(currentLine);
            currentLine.clear();
        }

        if (!currentLine.empty()) currentLine += " ";
        currentLine += word;
    } //uwrlin

    if (!currentLine.empty()) wrappedLines.push_back(currentLine);
    }//funct upd

    
}; //todo: modify to aggree with the global win manager inst


//track windows and their update intervals-used in window manager with data pushed from class window
struct WindowAndUpdateInterval {
    Window* window;   // Pointer to the window
    int updateTickMs; // Update interval in milliseconds

    // Constructor to initialize both values
    WindowAndUpdateInterval(Window* win): window(win), updateTickMs(win->UpdateTickMs) {} //fixed, valid update var tick

//TODO: this seems to have an issue where it doesn't notify window manager. it should notify window manager. also check for mem leaks

}; //todo: it's not taking class window right? 



//*************************************************************************************
//                        window manager
//handles windows and updating. just create a background osproscess with this in it. todo: how to run this

//how do i make sure theres only one instance
class WindowManager {
public:


// Returns pointer to the sole instance (creates if needed)
    static WindowManager* getInstance() {
        // Only create if graphics are enabled
        if (!AreGraphicsEnabled) {Serial.print("Error: Graphics not enabled. WindowManager will not start.");
        tft.setCursor(0,64);tft.fillScreen(0x0000);tft.print("err: graphics disabled"); //notify usr
            return nullptr;//really hope this doesn't cause a memeory leak
        }
        // Check if instance exists; if not, reinitialize it. todo: trigger if an attempt made to reference this while not around, currently not called as of 3/11/25
        if (!instance) {
            instance = new WindowManager();
        }
        return instance;
    }
    
    // Deletes the WindowManager instance after cleaning up windows.
    static void DestroyWindowManagerInst() {
        if (instance) {
            instance->deleteAllWindows();
            delete instance;
            instance = nullptr;
            isWindowHandlerAlive = false;
        }
    }


   // Global list of windows with their update intervals GLOBAL WINDOW REG
    std::vector<WindowAndUpdateInterval> windowRegistry; 

    // Register a new window
    void registerWindow(Window* win) {
        windowRegistry.emplace_back(win);  // Adds window with its update interval
    }

    // Unregister a window(&del)
void unregisterWindow(Window* win) { //takes ref to window
    if (!win) return; // Safety check

    // Clear the window from the screen
    tft.fillRect(win->config.x, win->config.y, win->config.width, win->config.height, 0x0000); // Black out area

    // Notify parent process if applicable
    if (win->config.hasParentProcess) {
        notifyParentProcess(win);  // Assume a function exists for this TODO: IMPLIMENT THIS!
    }

    // Remove from registry
    windowRegistry.erase(
        std::remove_if(windowRegistry.begin(), windowRegistry.end(),
            [&](const WindowAndUpdateInterval& entry) { return entry.window == win; }),
        windowRegistry.end());

    // Delete the window
    delete win;
    win = nullptr;
}




  void clearAllWindows() {
    for (auto& entry : windowRegistry) {
        tft.fillRect(entry.window->config.x, entry.window->config.y, entry.window->config.width, entry.window->config.height, 0x0000); // Black out each window
        delete entry.window;  // Delete each window
    }
    windowRegistry.clear();  // Remove all references

    tft.fillScreen(0x0000); // Black wipe the whole screen
    }



    // Get all windows (returns a vector of Window pointers)
    std::vector<Window*> getAllWindows() {
        std::vector<Window*> allWindows;
        for (auto& entry : windowRegistry) {
            allWindows.push_back(entry.window);
        }
        return allWindows; //ALL the windows
    }

    // Get window by name (returns a pointer to the window or nullptr if not found)
    Window* getWindowByName(const std::string& windowName) { //look through reg to find it
        for (auto& entry : windowRegistry) {
            if (entry.window->name == windowName) {
                return entry.window;
            }
        }
        return nullptr;  // If not found
    }

    // Update all windows based on their tick intervals
void updateAllWindows() {
    unsigned long currentTime = millis();

    for (auto it = windowRegistry.begin(); it != windowRegistry.end(); ) {
        Window* win = it->window;

        // Check if the window is null (invalid/dangling pointer)
        if (!win) {
            it = windowRegistry.erase(it);  // Remove invalid entry & continue
            continue; 
        }

        // Ensure the window is only updated on its interval & if it's dirty
        if (currentTime - win->lastUpdateTime >= win->UpdateTickMs) {  
            if (win->dirty) {  
                win->draw();
                win->lastUpdateTime = currentTime;  
            }
        }

        ++it; // Move iterator to next window (++it ignores old value for more efficiency)
    }
}

//what does this code do? makes it work. fuck you future me this is a level of headacheand migrane->
void WindowManager::notifyUpdateRateChange(Window* targetWindow, int newUpdateRate) { //find this shit and put the new update rate in 
    // Check if target window is in the registry
    for (auto& entry : windowRegistry) {
        if (entry.window == targetWindow) {
            // Found the window; change its update rate
            entry.updateTickMs = newUpdateRate;
            targetWindow->UpdateTickMs = newUpdateRate;
            Serial.print("Update rate changed for window: ");Serial.print(targetWindow->name);Serial.print(" to ");Serial.print(newUpdateRate);Serial.println("ms"); //i hate the way this looks and serial lib
            return;  // Exit after changing the update rate
        }
    }
    
    // If not found in the registry, handle this error
    Serial.print("Error: Window not found in registry for update rate change!"); //todo fail gracefully
}






  void selfDestructWinManager(){ //for when we need to delete the manager becausse some fuckshit is happening
DestroyWindowManagerInst();//destroy this and all sub windows
tft.setCursor(0,64);
tft.fillScreen(0x0000); 
tft.print("graphics disabled"); //put on screen and it should just stay like that till they restart
Serial.print("shutting down all the fucking graphics. hope you have something to restart it later");

  }


private:
    WindowManager() { //on create
        isWindowHandlerAlive = true;
        Serial.print("WindowManager created.\n"); 
    }
    ~WindowManager() { //on destroy
        Serial.print("WindowManager destroyed.\n");
        tft.setCursor(0,64);
        tft.fillScreen(0x0000); 
        tft.print("graphics proscess system disabled-winmgr_destr"); 
    }
    
    // Disable copy/assignment
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    static WindowManager* instance;
};

// Initialize the static member
WindowManager* WindowManager::instance = nullptr;





//tips: to get win by name use "Window* myWindow = windowManager.getWindowByName("Window1");" w/ the name :)

//to del a win, windowManager.unregisterWindow(myWindow);
//to wipe em all use windowManager.clearAllWindows();-should work and not leak memory









///***************************************************************************************************************************************************************************************************************************************************************
//operating system defaults for various types of screen setups
//default groupings are groups of default windows to put on the screen in some conditions, saving you time from having to manually add one of each window to the screen [lock screen,app screen, etc]
///***************************************************************************************************************************************************************************************************************************************************************

//NONE NOW, WE NOW USE APPLICATIONS FOR THIS

#endif
