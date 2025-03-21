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
 bool AreGraphicsEnabled = true;//userconfig
 bool isWindowHandlerAlive = false;//do not touch this in user code ever, for the window manager code only. "hey guys what if we could break the entire goddamn window manager on a whim!!!" "get the fuck outta my office"

//define it here, it's extern in main idk
//declare structs and stuff of the elements of this stupid thing.

// Window structure to hold window properties
struct WindowCfg {
    int x = 0, y = 0, width = 64, height = 64;
    bool auto_alignment=0,wrap=1; // Align text centrally or not,wrap text
    int textsize=1; //default text size inside this window
    bool borderless=false;
    uint16_t borderColor, bgColor, text_color; // Colors -add defaults
    uint16_t UpdateTickRate=500;//update every how many ms?
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

// Initialize the static window manager structure
static std::unique_ptr<WindowManager> WinManagerInstance;


//end forward dependencies



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
    std::shared_ptr<Window> parentWindow;  // Now shared to avoid ownership cycles
    
    // Constructor: initializes from a CanvasCfg structure and assigns parent pointer.
// Window owns Canvas, Canvas just references Window (not owning)
Canvas(const CanvasCfg& cfg, std::shared_ptr<Window> parent)
    : x(cfg.x), y(cfg.y), width(cfg.width), height(cfg.height),
      bgColor(cfg.bgColor), borderColor(cfg.borderColor), parentWindow(parent) {}

    // Clear the canvas area
    void clear() {
        // Fill the canvas area with the background color to clear it
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


//todo: add support for bitmap in an optimized manner

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
        if (canvasDirty) {
            unsigned long startTime = millis(); 
            tft.fillRect(x, y, width, height, bgColor);
            if (!borderless) {
                tft.drawRect(x, y, width, height, borderColor);
            }
            
            std::sort(drawElements.begin(), drawElements.end(), [](const DrawableElement &a, const DrawableElement &b) {
                return a.layer < b.layer;
            });

            for (auto &elem : drawElements) {
                elem.drawFunc();
            }

            lastFrameTime = millis() - startTime;
            canvasDirty = false;
        }
    }
    




    void update() { //only updates if dirty,chexks as to not waste frames
        unsigned long now = millis();
        if (canvasDirty || (now - lastUpdateTime >= updateIntervalMs)) {
            Draw();
            lastUpdateTime = now;
        }
    }

private:
    unsigned long lastUpdateTime = 0;
    unsigned int updateIntervalMs = 100;  // CONSISTENT NAME
    unsigned int lastFrameTime = 0;
};


//****************************************************************************************************************************************
//todo: frametime check doesn't seem to be correctly done,check if it's done right :)

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
    uint16_t UpdateTickRate=500;//update every how ms? 
*/



class Window : public std::enable_shared_from_this<Window> {
public:
    std::string name;       // Window's name
    WindowCfg config;       // Window configuration
    std::string content;    // Full text content (may be longer than visible area)

    // List of canvases attached to this window
    std::vector<std::shared_ptr<Canvas>> canvases; // Vector of smart pointers

  unsigned int updateIntervalMs = 100; // CONSISTENT VARIABLE NAME
  // update interval in ms-take from config now! yay i think
    bool dirty = false;     // uased as redraw flag
unsigned long lastUpdateTime = 0;  // in ms

unsigned int lastFrameTime = 0;    // duration of last draw

       int scrollOffsetX=0; //scroll offsets for these windows
        int scrollOffsetY=0;

    std::vector<std::string> wrappedLines;    // Wrapped text lines for rendering.  

    // Constructor
Window(const std::string& windowName, const WindowCfg& cfg, const std::string& initialContent = "")  : name(windowName), config(cfg), content(initialContent) {/*we wold put init content here but we have no need for that now!*/}

    // Destructor: Clean up canvases
~Window() {
    Callback2WinManager_window_deleted();  // Custom callback when the window is deleted
    // Any other custom cleanup tasks
}

void MoveWindow(int newX, int newY) { //mofe from old location to new
    if (config.x == newX && config.y == newY) return; // No change, no need to update

    config.x = newX;
    config.y = newY;
    
    forceUpdate(true); // Force a redraw since the position changed
}

void animateMove(int targetX, int targetY, int steps = 5) { //move the window but try to animate it
    int stepX = (targetX - config.x) / steps;
    int stepY = (targetY - config.y) / steps;
    
    for (int i = 0; i < steps; i++) {
        config.x += stepX;
        config.y += stepY;
        forceUpdate(false);
        delay(45); // Small delay to make animation visible
    }
    
    // Ensure final position is exact
    config.x = targetX;
    config.y = targetY;
    forceUpdate(true);
}

void ResizeWindow(int newWidth, int newHeight) { 
    if (config.x == newWidth && config.y == newHeight) return; // No change, no need to update

    config.width = newWidth;
    config.height = newHeight;
    
    forceUpdate(true); // Force a redraw since the size changed. 
}

void forceUpdate(bool updateSubComps) {//todo: toggle to NOT update offscreen canvas comps
    dirty = true;
    draw();  // Immediately update window content now, FUCKING RIGHT NOW
    if (updateSubComps) {
        for (auto& c : canvases) {
            // Force update on each canvas (even if off-screen or timer-based)
            if (c) c->update(); //push canvas update for iterator-this makes sure c-> update checks for valid first
        }
    }
}

void forceUpdateSubComps(){ //todo: toggle to NOT update offscreen canvas comps
  for (auto& c : canvases) {
            // Force update on each canvas (even if off-screen or timer-based)
            if (c) c->update(); //push canvas update for iterator -this makes sure c-> update checks for valid first
        }
}

void setUpdateTickRate(int newRate) {
    UpdateTickRate = newRate;
    WindowManager::getWinManagerInstance().notifyUpdateTickRateChange(this); //change the variable in the update rate
}



    // Add a canvas to this window
    void addCanvas(const CanvasCfg& cfg) {

        if (cfg.parentWindow != this) {
        Serial.print("Error: Canvas parent mismatch in window: "); Serial.println(name.c_str());
        //reject request, it's on the wrong line
            return;
        }
        //safe,create new canvas+pushback-smart ptr
    auto newCanvas = std::make_shared<Canvas>(cfg, shared_from_this());
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
// Global or member variables governming this window s scrolligng
int ScrollaccumDX = 0, ScrollaccumDY = 0;
uint64_t lastScrollTime = getCurrentTimeMillis(); //incorrectlynamed
const int scrollLimit = 3; // max scrolls per period
const int scrollPeriod = 100; // in ms



//new scroll code, now supporting directions
void WindowScroll(int DX, int DY) { //changes in directions
    // accumulate scroll deltas
accumDX += DX;
accumDY += DY;

    
    uint64_t now = getCurrentTimeMillis();
    if ( (now - lastScrollTime) >= scrollPeriod || abs(accumDX) >= scrollLimit || abs(accumDY) >= scrollLimit) { 
      //if you scroll a bunch of fucking times in a time period we will group scrolls together before applying them,preventing extra writes to vram. untested as of 3/17/2025

        // update scroll offsets
        scrollOffsetX += accumDX;
        scrollOffsetY += accumDY;
     int totalTextHeight = wrappedLines.size() * (8 * config.textsize);  // Total height of all the wrapped text
    int maxOffsetY = (totalTextHeight > config.height - 4) ? totalTextHeight - (config.height - 4) : 0;
    scrollOffsetY = std::max(0, std::min(scrollOffsetY, maxOffsetY));
    
    // Clamp horizontal scroll offset (if needed)
    int totalTextWidth = calculateTotalTextWidth();  // todo: add better logic for this so we can skip ahead in the text
    int maxOffsetX = (totalTextWidth > config.width - 4) ? totalTextWidth - (config.width - 4) : 0;
    scrollOffsetX = std::max(0, std::min(scrollOffsetX, maxOffsetX));
        
        // Notify components to update
        forceUpdate(true);//push an update right goddamn now and 2 the subcomps
        // Reset accumulators & update last time stamp
        accumDX = 0;
        accumDY = 0;
        lastScrollTime = now;
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

        lastFrameTime = millis() - startTime;
        dirty = false;

        // Update/draw each canvas only if it's at least partially within the window.
         for (auto& canvas : canvases) { // Iterate over all canvases
         if (canvas) { // Ensure the pointer is valid
            // Calculate the canvas's absolute position relative to the window.
            int canvasAbsX = config.x + c->x;
            int canvasAbsY = config.y + c->y;
            // Check if the canvas is completely off-screen relative to the window.
            if ((canvasAbsX + c->width) < config.x || canvasAbsX > (config.x + config.width) ||(canvasAbsY + c->height) < config.y || canvasAbsY > (config.y + config.height)) {
                //(if x> possible and y>possible)
                continue;  // Skip drawing this canvas if it's entirely off-screen.
            }
            c->update();  // Update/draw the canvas. 
        }//end if valid statement
    }//end canvas iterator
  }//end void draw





private:



void updateWrappedLines() {
    static std::string lastContent;
    static std::vector<std::string> lastWrappedLines;

    if (content == lastContent) return;
    lastContent = content;

    wrappedLines.clear();
    int charWidth = 6 * config.textsize;
    int maxCharsPerLine = (config.width - 4) / charWidth;

    std::string_view textView(content);
    std::string currentLine;
    
    size_t pos = 0;
    while (pos < textView.size()) {
        size_t nextSpace = textView.find(' ', pos);
        if (nextSpace == std::string::npos) nextSpace = textView.size(); //npos doesn't mean position with a typo, it means static member const value of elem w/ greatest size

        std::string_view word = textView.substr(pos, nextSpace - pos);
        pos = nextSpace + 1;  // Move past the space

        while (word.length() > maxCharsPerLine) { // Handle long words
            wrappedLines.emplace_back(word.substr(0, maxCharsPerLine));
            word.remove_prefix(maxCharsPerLine);
        }
        
        if (currentLine.length() + word.length() + 1 > maxCharsPerLine) {
            wrappedLines.push_back(currentLine);
            currentLine.clear();
        }
        if (!currentLine.empty()) currentLine += " ";
        currentLine += word;
    }
    if (!currentLine.empty()) wrappedLines.push_back(currentLine);

    if (wrappedLines != lastWrappedLines) {
        lastWrappedLines = wrappedLines;
        drawVisibleLines();
    }
}


void drawVisibleLines() {
    int charHeight = 8 * config.textsize;
    int startLine = scrollOffset / charHeight;
    int visibleLines = (config.height - 4) / charHeight;
    int y = config.y + 2 - (scrollOffset % charHeight);
    
    tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor); // Clear area
    
for (int i = startLine; i < wrappedLines.size() && i < startLine + visibleLines; i++) {
    tft.setCursor(config.x + 2, y);
    tft.print(wrappedLines[i].c_str());  // Convert std::string to const char* for the fucking adafruit
    y += charHeight;
}

}










}; //end window obj


//track windows and their update intervals-used in window manager with data pushed from class window
struct WindowAndUpdateInterval {
 std::weak_ptr<Window> window;
 int updateIntervalMs;

    WindowAndUpdateInterval(std::shared_ptr<Window> win): window(win), updateIntervalMs(win->updateIntervalMs) {} 

void updateIfValid() {
    if (auto winPtr = window.lock()) {  // Try to get a shared_ptr
        winPtr->update();//meowwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
    }
}

};


//*************************************************************************************
//                        window manager
//handles windows and updating. just create a background osproscess with this in it.
//do not create more than one object in main, i've got some code to reject creation of one already exists

class WindowManager {

public:
    WindowManager() { 
        isWindowHandlerAlive = true;
        Serial.print("WindowManager created.\n"); 
    }

    ~WindowManager() { 
        clearAllWindows();  // Properly clean up all windows
        WinManagerInstance = nullptr; // Nullify the singleton reference
        isWindowHandlerAlive = false;

        Serial.print("WindowManager destroyed.\n");
        tft.fillScreen(0x0000);
        tft.print("graphics system disabled");
    }//DEStructor



// Returns pointer to the sole WinManagerInstance (creates if needed)
    static WindowManager* getWinManagerInstance() {
        // Only create if graphics are enabled
        if (!AreGraphicsEnabled) {Serial.print("Error: Graphics not enabled. WindowManager will not start.");
        tft.setCursor(0,64);tft.fillScreen(0x0000);tft.print("err: graphics disabled"); //notify usr
            return nullptr;//really hope this doesn't cause a memeory leak
        }
        // Check if WinManagerInstance exists; if not, reinitialize it. todo: trigger construct if an attempt made to reference this while not around, currently not called as of 3/11/25
        if (!WinManagerInstance) {
            WinManagerInstance = new WindowManager();
        }
        return WinManagerInstance;
    }
    


   // Global list of windows with their update intervals GLOBAL WINDOW REG
    std::vector<std::shared_ptr<Window>> windowRegistry;


    // Register a new window
    void registerWindow(std::shared_ptr<Window> win) { 
    windows.push_back(win); // Adds window with its update interval and places it inside the storage
    }

    // Unregister a window(&del)
void unregisterWindow(Window* win) {
    if (!win) return;  // Safety check

    auto it = std::remove_if(windowRegistry.begin(), windowRegistry.end(),
        [&](const std::shared_ptr<Window>& w) { return w.get() == win; });

    if (it != windowRegistry.end()) {
        windowRegistry.erase(it, windowRegistry.end());
    }

    // Clear window from screen
    tft.fillRect(win->config.x, win->config.y, win->config.width, win->config.height, 0x0000);
}



void Callback2WinManager_window_deleted (){//todo: say which one
Serial.print("something deleted a window.\n"); }


void clearAllWindows() {
  for (auto& entry : windowRegistry) {
    tft.fillRect(entry.window->config.x, entry.window->config.y,entry.window->config.width, entry.window->config.height, 0x0000); 
    }
    windowRegistry.clear();  // Smart pointers clean up automatically FROM WINDOW REGISTRY
    tft.fillScreen(0x0000);  // Black wipe the whole screen
}





    // Get all windows (returns a vector of Window pointers)
std::vector<Window*> getAllWindows() {
    std::vector<Window*> allWindows;
    for (auto& entry : windowRegistry) {
        allWindows.push_back(entry.get());  // Use .get() to extract raw pointer
    }
    return allWindows;
}


    // Get window by name (returns a pointer to the window or nullptr if not found)
std::shared_ptr<Window> getWindowByName(const std::string& windowName) {
    for (auto& entry : windowRegistry) {
        if (auto winPtr = entry.window.lock()) {
    if (winPtr->name == windowName) {
        return winPtr;
        }
    }
    return nullptr;  // If not found
  }
}


    // Update all windows based on their tick intervals
void updateAllWindows() {
    unsigned long currentTime = millis();

    for (auto it = windowRegistry.begin(); it != windowRegistry.end(); ) {
        auto win = *it;  // win is now a shared_ptr<Window>

        if (!win) {
            it = windowRegistry.erase(it);  // Remove invalid entry & continue
            continue; 
        }

        if (currentTime - win->lastUpdateTime >= win->UpdateTickRate) {  
            if (win->dirty) {  
                win->draw();
                win->lastUpdateTime = currentTime;  
            }
        }

        ++it;
    }
}



void notifyUpdateTickRateChange(Window* targetWindow, int newUpdateTickRate) {
    for (auto& entry : windowRegistry) {
        if (entry.get() == targetWindow) {
            entry->UpdateTickRate = newUpdateTickRate;
            Serial.print("Update rate changed for window: ");
            Serial.print(" to ");
            Serial.print(newUpdateTickRate);
            Serial.println("ms");
            return;
        }
    }
    Serial.print("Error: Window not found in registry!");
}





void selfDestructWinManager(){ //for when we need to delete the manager becausse some fuckshit is happening
    WinManagerInstance = nullptr;

//TODO, MAKE SURE I'M PROPERLY KILLING SINGLETON
    clearAllWindows();
  tft.setCursor(0,64);
  tft.fillScreen(0x0000); 
  tft.print("graphics disabled"); //put on screen and it should just stay like that till they restart
  Serial.print("shutting down all the fucking graphics. hope you have something to restart it later. try not to call anything when it's shut down.");
  }


private:
//nothing really in private

};





//tips: to get win by name use "Window* myWindow = windowManager.getWindowByName("Window1");" w/ the name :)

//to del a win, windowManager.unregisterWindow(myWindow);
//to wipe em all use windowManager.clearAllWindows();-should work and not leak memory









///***************************************************************************************************************************************************************************************************************************************************************
//operating system defaults for various types of screen setups
//default groupings are groups of default windows to put on the screen in some conditions, saving you time from having to manually add one of each window to the screen [lock screen,app screen, etc]
///***************************************************************************************************************************************************************************************************************************************************************

//NONE NOW, WE NOW USE APPLICATIONS FOR THIS

#endif
