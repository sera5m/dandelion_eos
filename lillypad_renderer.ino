// LILLYPAD_RENDERER.h

#ifndef LILLYPAD_RENDERER_H
#define LILLYPAD_RENDERER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <memory>

//todo:

//little alert icons
//jpeg+png+bmp

//at attatches to canvas. vect draWings just use a few simple lines and cubes. we save them on the setup of the draWing object, with it storing the commands on start? idk how to do this outside of vague ideas.
//crude animations [involves placing objects on the canvas, with objects being an image or "generated vector draWing"] 

//optimize update cycle

//WrapText stuff around like text. just read uglib and use that stuff i guess. we can take heavy inspiration from the code.
//scroll boxes please
//dirty flag should be improved. needs tohave autoatic draw calls


//was gonna name this orchid renderer but some guy made a render engine already named that and it was cool tbh



//this is the Window handler for dandelionn. if you want low level stuff about the screen see module_math_render_base. that module handles direct controll of the screen
//this module handles Window creation and draw calls. version 4, now with performance enhancements. :3
//i do not know what version oim on anymore nad fu in need to die

#include "module_math_render_base.ino" //i hate this verbose name
extern Adafruit_SSD1351 tft;  /* fucking shit needs to be imported. why is this not treated as a global object from the fucking screen setup*/


//forward declare all possible dependencies/child of Window before we use them

class Canvas;  // forward declaration 
class Window; //do i have to define the Window class?
class WindowManager; //only make one of these on os start
//include the struct dependencies of children

//userspace graphics config import do not delete this or the whole system will fuck up
 bool AreGraphicsEnabled = true;//userconfig
 bool IsWindowHandlerAlive = false;//do not touch this in user code ever, for the Window manager code only. "hey guys what if we could break the entire goddamn Window manager on a whim!!!" "get the fuck outta my office"

//define it here, it's extern in main idk
//declare structs and stuff of the elements of this stupid thing.

// Window structure to hold Window properties
struct WindowCfg {
    int x = 0, y = 0, width = 64, height = 64;
    bool AutoAlignment=0,WrapText=1; // Align text centrally or not,WrapText text
    int TextSize=1; //default text size inside this Window
    bool borderless=false;
    uint16_t BorderColor, bgColor, WinTextColor; // Colors -add defaults
    uint16_t UpdateTickRate=500;//update every how many ms?
}; 



struct CanvasCfg {
    int x = 0, y = 0, width = 32, height = 32;
    bool borderless=true; 
    bool DrawBG=true;
     uint16_t bgColor = 0x0000, BorderColor = 0xFFFF; 
      Window* parentWindow = nullptr;
};




void CanvasForceParentUpdate(std::shared_ptr<Window> p);//forward slop dependancy 4 this, do not edit pls



struct PixelDat{ //wanna draw a shitton of individual pixels with no care for efficiency? (i assume used for graphing)
int posX;
int posY;
uint16_t color;
int layer;
};

// Initialize the static Window manager structure
static std::unique_ptr<WindowManager> WinManagerInstance;


//end forward dependencies



//****************************************************************************************************************************************

//canvas draw logic
//canvases are pannels that you can draw various things in, and they'll keep it from spilling off. great for doing stuff like Windows

//******************************************************************************************************************************************************


// A DrawableElement represents any element (text or shape) drawn on the canvas.
struct DrawableElement {
    uint layer;  // Lower layer drawn first, higher layers drawn on top
    std::function<void()> drawFunc; // Lambda that draws this element
};



// Canvas class definition

class Canvas {
private:
    unsigned long lastUpdateTime = 0;
    unsigned int UpdateTickRate = 100;  // CONSISTENT NAME
    unsigned int lastFrameTime = 0;
    
public:
    // Position and size of the canvas on screen (relative to parent Window if needed)
    int x, y, width, height;
    uint16_t bgColor, BorderColor;
    
    // Flag to indicate if canvas needs updating.
    bool canvasDirty = true;
    bool borderless;  //now takes borderless from struct
    bool DrawBG=true;
    // Container for drawable elements on this canvas.
    std::vector<DrawableElement> drawElements; //replace with function ptr in future
    
    // Reference to parent Window (if any)
    std::shared_ptr<Window> parentWindow;  // Now shared to avoid ownership cycles
    
    // Constructor: initializes from a CanvasCfg structure and assigns parent pointer.
// Window owns Canvas, Canvas just references Window (not owning)
//for reference when working with this code,
/*struct CanvasCfg {
    int x = 0, y = 0, width = 32, height = 32;
    bool borderless=true; 
    bool DrawBG=true;
     uint16_t bgColor = 0x0000, BorderColor = 0xFFFF; 
      Window* parentWindow = nullptr;
};*/

Canvas(const CanvasCfg& cfg, std::shared_ptr<Window> parent)
    : x(cfg.x), y(cfg.y), width(cfg.width), height(cfg.height), borderless(cfg.borderless),DrawBG(cfg.DrawBG), bgColor(cfg.bgColor), BorderColor(cfg.BorderColor), parentWindow(parent) {}

    // Clear the canvas area
    void clear() { //todo: modify for backgroundless 
        // Fill the canvas area with the background color to clear it
        // Note: if you need clipping, ensure tft supports it or implement it yourself,dear user.
        if(DrawBG) {
        tft.fillRect(x, y, width, height, bgColor);
        canvasDirty = true;
        }
        else{
            //put some kind of logic to force parent to update?
          // In clear():
            CanvasForceParentUpdate(parentWindow);
          //this is the worst fucking way to do this. but i can't un-draw shit. unsure what to do here because transparency is...evil and impossible for DMAA i think.
        }
    }
    
    // Adds a text line to the canvas.
    // (No WrapTextping—if the text extends beyond the canvas, it is simply clipped.)
    void AddTextLine(int posX, int posY, const String &text, uint8_t txtsize, uint16_t WinTextColor, int layer = 0) {
        DrawableElement element;
        element.layer = layer;

    element.drawFunc = [text,txtsize, this, posX, posY, WinTextColor]() {//force explicit copy for persistance-these xy coords are for the canvas pos
    tft.setTextColor(WinTextColor); // Remove .get()
    tft.setTextSize(txtsize);
    int16_t Textx, Texty; //local vars for position of text in canvas
    //tft.getCursor(&x, &y); //there's no function named get cursor. it would be nice if it EXISTED!!!
    tft.setCursor(Textx + posX, Texty + posY);
    tft.print(text.c_str());
    };//end elem drawfunc


        drawElements.push_back(element);
        canvasDirty = true;
    }//end addtextline




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
    };//end drawfunc

  drawElements.push_back(element);
  canvasDirty = true;
}
  //adafruit has optimized line draWing and normal line draWing, for an angular one it's drawLine,
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


 // draWing a rectangle.

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

//this is auto gennerated. fuck you. 
void AddBitmap(int posX, int posY, const uint16_t* bitmap, int w, int h, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    
    element.drawFunc = [=, this]() { 
        tft.drawRGBBitmap(x + posX, y + posY, bitmap, w, h);
    };

    drawElements.push_back(element);
    canvasDirty = true;
}





//back to main draw logic for canvas here



    // Draws the canvas: first draws the canvas background (and border if not borderless),
    // then sorts and draws the drawable elements by layer.
        void CanvasDraw() {
        
            unsigned long startTime = millis(); 
            if(!DrawBG){tft.fillRect(x, y, width, height, bgColor);} //if not backgroundless,draw this-todo:this is BAD and the refresh color will still be left behind!
            if (!borderless) {
                tft.drawRect(x, y, width, height, BorderColor);
            }
            
            std::sort(drawElements.begin(), drawElements.end(), [](const DrawableElement &a, const DrawableElement &b) {
                return a.layer < b.layer;
            });

            for (auto &elem : drawElements) {
                elem.drawFunc();
            }

            lastFrameTime = millis() - startTime;
            
        
    }
    




    void CanvasUpdate(bool force) { //only updates if dirty,chexks as to not waste frames
        unsigned long now = millis();
        if (canvasDirty || (now - lastUpdateTime >= UpdateTickRate)|| force) {
            CanvasDraw();
            lastUpdateTime = now;
            canvasDirty = false;
        }
    }


};


//****************************************************************************************************************************************
//todo: frametime check doesn't seem to be correctly done,check if it's done right :)

//Window draw logic

//Windows are pannels that you can have text and attatch to a proscess to directly write to or modify. all children like widgets or canvas pannels need to be attatched to Windows.
//Windows are what you call update on, so they update sub elements

//******************************************************************************************************************************************************
//remember, constructor uses 
/*struct WindowCfg 
    int x = 0, y = 0, width = 64, height = 64;
    bool AutoAlignment=0,WrapText=1; // Align text centrally or not,WrapText text
    int TextSize=1; //default text size inside this Window
    bool borderless=false;
    uint16_t BorderColor, bgColor, WinTextColor; // Colors -
    uint16_t UpdateTickRate=500;//update every how ms? 
*/



class Window : public std::enable_shared_from_this<Window> {
public:
    std::string name;       // Window's name
    WindowCfg config;       // Window configuration
    std::string content;    // Full text content (may be longer than visible area)
    std::vector<std::string> wrappedLines;    // Wrapped text lines for rendering.  todo:move private

    // List of canvases attached to this Window
    std::vector<std::shared_ptr<Canvas>> canvases; // Vector of smart pointers

  unsigned int UpdateTickRate = 100; // CONSISTENT VARIABLE NAME
  // update interval in ms-take from config now! yay i think
    bool dirty = false;     // uased as redraw flag
unsigned long lastUpdateTime = 0;  // in ms
unsigned int lastFrameTime = 0;    // duration of last draw

    bool IsWindowShown = true;//windows are shown by default on creation

       int scrollOffsetX=0; //scroll offsets for these Windows //TODO:MOVE THESE TO PRIVATE
       int scrollOffsetY=0;

      int accumDX = 0, accumDY = 0; // member var for the stupid scroll func: todo: move to private, this is awful



    // Constructor
Window(const std::string& WindowName, const WindowCfg& cfg, const std::string& initialContent = "")  : name(WindowName), config(cfg), content(initialContent) {/*we wold put init content here but we have no need for that now!*/}

    // Destructor: Clean up canvases
~Window() {
    //Callback2WinManager_Window_deleted();  // Custom callback when the Window is deleted
    // Any other custom cleanup tasks
}

void ForceUpdate(bool UpdateSubComps) {//todo: toggle to NOT update offscreen canvas comps-this updates em all by force
    dirty = true;
    WinDraw();  // Immediately update Window content now, FUCKING RIGHT NOW 
    if (UpdateSubComps) {
        for (auto& canvas : canvases) {
            // Force update on each canvas (even if off-screen or timer-based)
            if (canvas) canvas->CanvasUpdate(UpdateSubComps); //push canvas update for iterator-this makes sure canvas-> update checks for valid first
        }
    }
}

void MoveWindow(int newX, int newY) { //mofe from old location to new
    if (config.x == newX && config.y == newY) return; // No change, no need to update

    config.x = newX;
    config.y = newY;
    
    ForceUpdate(true); // Force a redraw since the position changed
}

void animateMove(int targetX, int targetY, int steps = 5) { //move the Window but try to animate it
    int stepX = (targetX - config.x) / steps;
    int stepY = (targetY - config.y) / steps;
    
    for (int i = 0; i < steps; i++) {
        config.x += stepX;
        config.y += stepY;
        ForceUpdate(false);
        delay(45); // Small delay to make animation visible
    }
    
    // Ensure final position is exact
    config.x = targetX;
    config.y = targetY;
    ForceUpdate(true);
}

void ResizeWindow(int newWidth, int newHeight) { 
    if (config.width == newWidth && config.height == newHeight) return; // No change, no need to update

    config.width = newWidth;
    config.height = newHeight;
    
    ForceUpdate(true); // Force a redraw since the size changed. 
}



void ForceUpdateSubComps(){ //todo: toggle to NOT update offscreen canvas comps
  for (auto& canvas : canvases) {
            // Force update on each canvas (even if off-screen or timer-based)
            if (canvas) canvas->CanvasUpdate(true); //push canvas update for iterator -this makes sure canvas-> update checks for valid first
        }
}
/*
void setUpdateTickRate(int newRate) {
    UpdateTickRate = newRate;
    WindowManager::getWinManagerInstance().notifyUpdateTickRateChange(this); //change the variable in the update rate
}*/



    // Add a canvas to this Window
    void addCanvas(const CanvasCfg& cfg) {

        if (cfg.parentWindow != this) {
        Serial.print("Error: Canvas parent mismatch in Window: "); Serial.println(name.c_str());
        //reject request, it's on the wrong line
            return;
        }
        //safe,create new canvas+pushback-smart ptr
    auto newCanvas = std::make_shared<Canvas>(cfg, shared_from_this());
    canvases.push_back(newCanvas);
    }


    // Update the Window's content and mark as dirty
    void updateContent(const std::string& newContent) {
        if (content != newContent) {
            content = newContent;
            dirty = true;
            WinDraw(); // immediate redraw
        }
    }
// Global or member variables governming this Window s scrolligng
int ScrollaccumDX = 0, ScrollaccumDY = 0;
uint64_t lastScrollTime = millis(); 
const int scrollLimit = 3; // max scrolls per period
const int scrollPeriod = 100; // in ms

int calculateTotalTextWidth() {
    int charWidth = 6 * config.TextSize; // Assuming fixed-width font
    int maxWidth = 0;
    for (const auto& line : wrappedLines) {
        int lineWidth = line.length() * charWidth;
        if (lineWidth > maxWidth) {
            maxWidth = lineWidth;
        }
    }
    return maxWidth;
}

//new scroll code, now supporting directions
void WindowScroll(int DX, int DY) { //changes in directions
    // accumulate scroll deltas
accumDX += DX;
accumDY += DY;

    
    uint64_t now = millis();
    if ( (now - lastScrollTime) >= scrollPeriod || abs(accumDX) >= scrollLimit || abs(accumDY) >= scrollLimit) { 
      //if you scroll a bunch of fucking times in a time period we will group scrolls together before applying them,preventing extra writes to vram. untested as of 3/17/2025

        // update scroll offsets
        scrollOffsetX += accumDX;
        scrollOffsetY += accumDY;
     int totalTextHeight = wrappedLines.size() * (8 * config.TextSize);  // Total height of all the wrapped text
    int maxOffsetY = (totalTextHeight > config.height - 4) ? totalTextHeight - (config.height - 4) : 0;
    scrollOffsetY = std::max(0, std::min(scrollOffsetY, maxOffsetY));
    
    // Clamp horizontal scroll offset (if needed)
    int totalTextWidth = calculateTotalTextWidth();  // todo: add better logic for this so we can skip ahead in the text
    int maxOffsetX = (totalTextWidth > config.width - 4) ? totalTextWidth - (config.width - 4) : 0;
    scrollOffsetX = std::max(0, std::min(scrollOffsetX, maxOffsetX));
        
        // Reset accumulators & update last time stamp BEFORE checking limits
        lastScrollTime = now; //we just scrolled
        int prevOffsetX = scrollOffsetX;
        int prevOffsetY = scrollOffsetY;
        accumDX = 0; 
        accumDY = 0;

        // Notify components to update only if scroll changed
        if (scrollOffsetX != prevOffsetX || scrollOffsetY != prevOffsetY) {
            ForceUpdate(true); // push an update RIGHT FUCKING NOW
          
        }

    }
}




    // Draws the Window and its text content, then updates child canvases if they're visible.
    void WinDraw() {
if (IsWindowShown) {//if the window is shown,draw this
          unsigned long startTime = millis();//framerate counter, time counter start
        // Clear the Window area
        tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor); //fill the Window with the background color
        if (!config.borderless) //if Window not borderless
            tft.drawRect(config.x, config.y, config.width, config.height, config.BorderColor); //draw the rectangle outline

        // Set text properties
        tft.setTextColor(config.WinTextColor);
        tft.setTextSize(config.TextSize);

        // Update wrapped lines from full content
        updateWrappedLines(); 

        lastFrameTime = millis() - startTime;
        dirty = false;

        // Update/draw each canvas only if it's at least partially within the Window.
         for (auto& canvas : canvases) { // Iterate over all canvases
         if (canvas) { // Ensure the pointer is valid
            // Calculate the canvas's absolute position relative to the Window.
            int canvasAbsX = config.x + canvas->x;
            int canvasAbsY = config.y + canvas->y;
            // Check if the canvas is completely off-screen relative to the Window.
            if ((canvasAbsX + canvas->width) < config.x || canvasAbsX > (config.x + config.width) ||(canvasAbsY + canvas->height) < config.y || canvasAbsY > (config.y + config.height)) {
                //(if x> possible and y>possible)
                continue;  // Skip draWing this canvas if it's entirely off-screen.
            }//end canvas check statement
            canvas->CanvasUpdate(true);  // Update/draw the canvas. todo: do a better method than force
        }//end if valid statement
    }//end canvas iterator
  }//end of is window shown?
    else { Serial.println("Window hidden (IsWindowShown = false)");  } //you never know. probably should remove later but... debug print statements are important
 }//end void draw


 void HideWindow() {
  IsWindowShown = false; //note:this automatically will stop void winupdate from updating even when called: because we have the flag in THERE
  tft.fillRect(config.x, config.y, config.width, config.height,0x0000);//fill the area with blackd

        // Optionally, instruct sub-elements (Canvases) to hide themselves.-needs a fix later
        // for (auto& canvas : Canvases) { if (canvas) canvas->Hide(); }
    }//end hide win

 void ShowWindow() {
  IsWindowShown = true;
  ForceUpdate(true);
        // Optionally, instruct all sub-elements (Canvases) to show themselves.
    }//end show window




private:
//defaults for delimiters for orderlyness
std::string Delim_LinBreak ="<b>";
std::string Delim_Seperator ="<_>";
//properties
std::string Delim_ColorChange = "<setcolor(";
std::string Delim_PosChange = "<pos(";
std::string Delim_Sizechange = "<textsize(";
//properties of special text
std::string Delim_Strikethr = "<s>";        // Italic text 
std::string Delim_Underline = "<u>";  // Underline text 

// ================== drawVisibleLines ==================
void drawVisibleLines() {
  Serial.println("start");
  if (IsWindowShown) {
    int baseTextSize = config.TextSize; // Store base text size
    uint16_t currentColor = config.WinTextColor; // Track color state
    int currentTextSize = baseTextSize; // Track text size state
    uint16_t originalWinTexColor = config.WinTextColor;//original window text color to res back 2
    int charHeight = 8 * currentTextSize;
    int charWidth = 6 * currentTextSize;
    int startLine = scrollOffsetY / charHeight;
    int visibleLines = (config.height - 4) / charHeight;
    int y = config.y + 2 - (scrollOffsetY % charHeight); //y set here

    tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor);

    for (int i = startLine; i < wrappedLines.size() && i < startLine + visibleLines; i++) {
      std::string line = wrappedLines[i];
      int startChar = max(0, scrollOffsetX / charWidth);
      int visibleChars = (config.width - 4) / charWidth;

      tft.setTextSize(currentTextSize);
      tft.setTextColor(currentColor);

      int cursorX = config.x + 2;
      int cursorY = y; //y is set above 
      tft.setCursor(cursorX, cursorY);

      int charsProcessed = 0;
      size_t pos = startChar;

      while (pos < line.length() && charsProcessed < visibleChars) {
        if (line[pos] == '<') {
          size_t end = line.find('>', pos);
          if (end != std::string::npos) {
            std::string tag = line.substr(pos, end - pos + 1);
            
            // Handle tags
            if (tag == Delim_LinBreak) { // Line break
              break; // Already wrapped, just stop processing this line
            }
            else if (tag.substr(0,5) == Delim_PosChange) {
              size_t comma = tag.find(',');
              if (comma != std::string::npos) { 
                int x = std::stoi(tag.substr(5, comma - 5));
                int y = std::stoi(tag.substr(comma+1, tag.find(')') - comma - 1));
                cursorX = config.x + x;
                cursorY = config.y + y;
                tft.setCursor(cursorX, cursorY);
              }
            }
else if (tag.substr(0, Delim_ColorChange.length()) == Delim_ColorChange) { 
    size_t start = tag.find('(') + 1; // Find '(' and move past it
    size_t end = tag.find(')');       // Find ')'
    
    if (start != std::string::npos && end != std::string::npos && end > start) { 
        std::string colorStr = tag.substr(start, end - start); // Extract the hex string
        printf("Extracted Color String: " ); // Debug

        uint32_t rawColor = std::stoul(colorStr, nullptr, 16); // Convert to number
        uint16_t color16 = rawColor & 0xFFFF; // Mask to 16-bit
        tft.setTextColor(color16);
    } else {
        printf( "Error: Malformed color tag");
    }
}



            else if (tag == Delim_Underline) {
              int lineY = cursorY + charHeight - 1;
              tft.drawFastHLine(cursorX, lineY, 
                (visibleChars - charsProcessed) * charWidth, currentColor);
            }
            else if (tag == Delim_Strikethr) {
              int lineY = cursorY + (charHeight / 2);
              tft.drawFastHLine(cursorX, lineY,
                (visibleChars - charsProcessed) * charWidth, currentColor);
            }


            else if (tag == Delim_Seperator) { // Separator
              tft.print(' ');
              charsProcessed++;//you may need to add the chars in the actual delim seperator len soooo
              cursorX += charWidth;
            }


            pos = end + 1; // Skip tag
          }
          else { // Malformed tag
            tft.print(line[pos]);
            pos++;
            charsProcessed++;
            cursorX += charWidth;
          }
        }
        else { // Normal character
          tft.print(line[pos]);
          pos++;
          charsProcessed++;
          cursorX += charWidth;
        }

        // Handle line overflow
        if (cursorX > config.x + config.width - 2) break;
      }

      y += charHeight;
      currentTextSize = baseTextSize; // Reset to window's base text size
      tft.setTextSize(currentTextSize);
      tft.setTextColor(originalWinTexColor); 
    }
  }
  Serial.println("end");
}



// ================== updateWrappedLines ==================
void updateWrappedLines() {
  static std::string lastContent;
  static std::vector<std::string> lastWrappedLines;

  if (content == lastContent) return;
  lastContent = content;

  wrappedLines.clear();
  int baseCharWidth = 6 * config.TextSize;
  int maxCharsPerLine = (config.width - 4) / baseCharWidth;

  std::string currentLine;
  size_t pos = 0;
  int currentLineLength = 0;

  while (pos < content.length()) {
    if (content[pos] == '<') {
      size_t end = content.find('>', pos);
      if (end != std::string::npos) {
        std::string tag = content.substr(pos, end - pos + 1);
        
        // Handle special tags
        if (tag == Delim_LinBreak) { // Hard line break
          wrappedLines.push_back(currentLine);
          currentLine.clear();
          currentLineLength = 0;
        }
        else if (tag == Delim_Seperator) { // Separator
          if (currentLineLength + 1 <= maxCharsPerLine) {
            currentLine += ' ';
            currentLineLength++;
          }
          else {
            wrappedLines.push_back(currentLine);
            currentLine = ' ';
            currentLineLength = 1;
          }
        }
        else if (tag == Delim_Underline || tag == Delim_Strikethr) {
          currentLine += tag; // Preserve formatting tags
        }
        else { // Other tags
          currentLine += tag;
        }
        
        pos = end + 1;
      }
      else { // Malformed tag
        currentLine += content[pos];
        currentLineLength++;
        pos++;
      }
    }
    else { // Normal character
      if (content[pos] == ' ') { // Word wrap point
        if (currentLineLength + 1 > maxCharsPerLine) {
          wrappedLines.push_back(currentLine);
          currentLine.clear();
          currentLineLength = 0;
        }
        else {
          currentLine += ' ';
          currentLineLength++;
        }
      }
      else { // Regular character
        if (currentLineLength >= maxCharsPerLine) {
          wrappedLines.push_back(currentLine);
          currentLine.clear();
          currentLineLength = 0;
        }
        currentLine += content[pos];
        currentLineLength++;
      }
      pos++;
    }
  }

  if (!currentLine.empty()) {
    wrappedLines.push_back(currentLine);
  }

  if (wrappedLines != lastWrappedLines) {
    lastWrappedLines = wrappedLines;
    drawVisibleLines();
  }
}//end update wrap lines


}; //end Window obj


//shitty workaround for canvas to force the parent to update
void CanvasForceParentUpdate(std::shared_ptr<Window> p){
  p->ForceUpdate(false); //do not update subcomps to avoid infinite loop. this should say false. DO NOT EDIT THIS CODE
}






struct WindowAndUpdateInterval {
    std::weak_ptr<Window> windowWeakPtr; // Renamed to avoid conflict
    int UpdateTickRate;

    // Constructor takes a shared_ptr and stores it as a weak_ptr
    WindowAndUpdateInterval(std::shared_ptr<Window> Win) 
        : windowWeakPtr(Win), UpdateTickRate(Win->UpdateTickRate) {}

    void updateIfValid() {
        if (auto WinPtr = windowWeakPtr.lock()) {  // Try to get a shared_ptr from weak_ptr
            WinPtr->WinDraw(); // Call WinDraw() on the Window
        }
    }
};


//*************************************************************************************
//                        Window manager
//handles Windows and updating. just create a background osproscess with this in it.
//do not create more than one object in main, i've got some code to reject creation of one already exists

class WindowManager {

public:
    WindowManager() { 
        IsWindowHandlerAlive = true;
        Serial.print("WindowManager created.\n"); 
    }

    ~WindowManager() { 
        clearAllWindows();  // Properly clean up all Windows
        WinManagerInstance = nullptr; // Nullify the singleton reference
        IsWindowHandlerAlive = false;

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
            WinManagerInstance = std::make_unique<WindowManager>();
        }
        return WinManagerInstance.get();
    }
    


    std::vector<WindowAndUpdateInterval> WindowRegistry;

    // Register a new Window
    void registerWindow(std::shared_ptr<Window> Win) {
        WindowRegistry.emplace_back(Win); // Create a WindowAndUpdateInterval object //todo error this needs to have a defa ult rateset
    }

    // Unregister a Window (& delete)
        void unregisterWindow(Window* Win) {
        if (!Win) return;  // Safety check

        auto it = std::remove_if(WindowRegistry.begin(), WindowRegistry.end(),
            [&](const WindowAndUpdateInterval& entry) {
                if (auto winPtr = entry.windowWeakPtr.lock()) {
                    return winPtr.get() == Win;
                }
                return false;
            });

        if (it != WindowRegistry.end()) {
            WindowRegistry.erase(it, WindowRegistry.end());
        }

        // Clear Window from screen
        tft.fillRect(Win->config.x, Win->config.y, Win->config.width, Win->config.height, 0x0000);
    }

/*
void Callback2WinManager_Window_deleted (){//todo: say which one
Serial.print("something deleted a Window.\n"); }
*/

 // Clear all Windows
    void clearAllWindows() {
        for (auto& entry : WindowRegistry) {
            if (auto winPtr = entry.windowWeakPtr.lock()) {
                tft.fillRect(winPtr->config.x, winPtr->config.y, winPtr->config.width, winPtr->config.height, 0x0000);
            }
        }
        WindowRegistry.clear();  // Smart pointers clean up automatically
        tft.fillScreen(0x0000);  // Black wipe the whole screen
    }

    // Get Window by name
    std::shared_ptr<Window> GetWindowByName(const std::string& WindowName) {
        for (auto& entry : WindowRegistry) {
            if (auto winPtr = entry.windowWeakPtr.lock()) {
                if (winPtr->name == WindowName) {
                    return winPtr;
                }
            }
        }
        return nullptr;  // If not found
    }

    // Other members...


    // Update all Windows based on their tick intervals
void UpdateAllWindows(bool Force,bool AndSubComps) {
    unsigned long currentTime = millis();

    for (auto it = WindowRegistry.begin(); it != WindowRegistry.end(); ) {
        if (auto winPtr = it->windowWeakPtr.lock()) {
            if (Force || (currentTime - winPtr->lastUpdateTime >= winPtr->UpdateTickRate)) {
                if (winPtr->dirty || Force) {  // Check both conditions
                    winPtr->WinDraw();
                    if (AndSubComps){winPtr->ForceUpdateSubComps(); }
                    winPtr->lastUpdateTime = currentTime;
                    winPtr->dirty = false;  // Clear dirty flag after drawing
                }
            }
            ++it;
        } else {
            it = WindowRegistry.erase(it);  // Remove invalid entry
        }
    }
}





void notifyUpdateTickRateChange(Window* targetWindow, int newUpdateTickRate) {
    for (auto& entry : WindowRegistry) {
        if (auto winPtr = entry.windowWeakPtr.lock()) {
            if (winPtr.get() == targetWindow) {
                winPtr->UpdateTickRate = newUpdateTickRate;
                Serial.print("Update rate changed for Window: ");
                Serial.print(" to ");
                Serial.print(newUpdateTickRate);
                Serial.println("ms");
                return;
            }
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
//nothing in private. because we have nothing to hide. I LOVE THE GOVERNMENT I LOVE THE GOVERNMENT I LOVE THE GOVERNMENT PLEASE HIRE ME PLEASE I WANT MONEY

};//end winmanager




//tips: to get Win by name use "Window* myWindow = WindowManager.getWindowByName("Window1");" w/ the name :)

//to del a Win, WindowManager.unregisterWindow(myWindow);
//to wipe em all use WindowManager.clearAllWindows();-should work and not leak memory









///***************************************************************************************************************************************************************************************************************************************************************
//operating system defaults for various types of screen setups
//default groupings are groups of default Windows to put on the screen in some conditions, saving you time from having to manually add one of each Window to the screen [lock screen,app screen, etc]
///***************************************************************************************************************************************************************************************************************************************************************

//NONE NOW, WE NOW USE APPLICATIONS FOR THIS

#endif
