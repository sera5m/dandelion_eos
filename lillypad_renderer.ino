// LILLYPAD_RENDERER.h

#ifndef LILLYPAD_RENDERER_H
#define LILLYPAD_RENDERER_H



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
      Window* parentWindow = nullptr;
};



struct PixelDat{ //wanna draw a shitton of individual pixels with no care for efficiency? (i assume used for graphing)
int posX;
int posY;
uint16_t color;
int layer;
};



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
        element.drawFunc = [=]() { //call this to draw

        // Check if it's a vertical line
        if (posX0 == posX1) {
            tft.drawFastVLine(x + posX0, y + std::min(posY0, posY1), std::abs(posY1 - posY0), color);
        }
        // Check if it's a horizontal line
        else if (posY0 == posY1) {
            tft.drawFastHLine(x + std::min(posX0, posX1), y + posY0, std::abs(posX1 - posX0), color);
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
        element.drawFunc = [=]() { //call this to draw
        tft.drawPixel(x + posX, y + posY, color); //args, posx posy, color
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }



//TODO: SHOULD HAVE PROPER IMPLIMENTATION FROM STRUCT PIXELDAT
//draw multiple pixels on multiple layers
    void AddPixels(int posX, int posY, int w, int h, uint16_t color, int layer = 0) { //replace with an array here
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
        tft.drawPixel(x + posX, y + posY, color); //args, posx posy, color //replace with for array
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    


 // drawing a rectangle.

//draw filled rect
    void AddFRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
        tft.fillRect(x + posX, y + posY, w, h, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
//draw a rect, not filled.
    void AddRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
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
        element.drawFunc = [=]() { //call this to draw
        tft.fillRoundRect(x + posX, y + posY, w, h, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
//draw a rounded rect, not filled.
    void AddRRect(int posX, int posY, int w, int h,uint16_t r, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
        tft.drawRoundRect(x + posX, y + posY, w, h, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }




//tri,hollow
    void AddTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, int layer = 0) { //args take one position per edge
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
         tft.drawTriangle(x0+x,y0+y,x1+x,y1+y,x2+x,y2+y, color); //positions of parent adn position of the 3 points taken 
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }


//filled triangle
    void AddFTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, int layer = 0) { //args take one position per edge
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
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
        element.drawFunc = [=]() { //call this to draw
        tft.fillCircle(x + posX, y + posY, r, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
//draw a ciurcle, not filled.
    void AddCircle(int posX, int posY, int r, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=]() { //call this to draw
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
    tft.DrawRect(x, y, width, height, borderColor);
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
    
    // A simple function to determine borderless state.
    bool borderless() const {
        // If borderless is desired, return true.
        // (Alternatively, you can store a borderless flag in this class.)
        return false; // For now, assume border should be drawn.
    }
    
    // Optionally update canvas only if dirty
    void update() {
    unsigned long now = millis();
    if (canvasDirty || (now - lastUpdateTime >= updateInterval)) {
        draw();
        lastUpdateTime = now;
    }
}


 private:

unsigned long lastUpdateTime = 0;   // in ms
unsigned int updateInterval = 100;    // default update rate (ms) for canvas
unsigned int lastFrameTime = 0;       // duration of last canvas draw



};



//****************************************************************************************************************************************


//window draw logic

//windows are pannels that you can have text and attatch to a proscess to directly write to or modify. all children like widgets or canvas pannels need to be attatched to windows.
//windows are what you call update on, so they update sub elements

//******************************************************************************************************************************************************

class Window {
public:
    std::string name;       // Window's name
    WindowCfg config;       // Window configuration
    std::string content;    // Full text content (may be longer than visible area)

    // List of canvases attached to this window
    std::vector<Canvas*> canvases;

    int WinUpdateMS = 500;  // update interval in ms
    bool dirty = false;     // needs redraw flag

    // NEW: Scrolling members:
    int scrollOffset = 0;                // Vertical scroll offset in pixels
    std::vector<String> wrappedLines;    // Wrapped text lines for rendering

    // Constructor
    Window(const std::string& windowName, const WindowCfg& cfg, const std::string& initialContent = "")
      : name(windowName), config(cfg), content(initialContent) {}

    // Destructor: Clean up canvases
    ~Window() {
        for (Canvas* canvas : canvases) {
            delete canvas;
        }
    }

    // Add a canvas to this window
    void addCanvas(const CanvasCfg& cfg) {
        if (cfg.parentWindow != this) {
            Serial.println("Error: Canvas parent does not match this window.");
            return;
        }
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
          unsigned long startTime = millis();
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
            if ((canvasAbsX + c->width) < config.x || canvasAbsX > (config.x + config.width) ||
                (canvasAbsY + c->height) < config.y || canvasAbsY > (config.y + config.height)) {
                continue;  // Skip drawing this canvas if it's entirely off-screen.
            }
            c->update();  // Update/draw the canvas.
        }
    }

private:

unsigned long lastUpdateTime = 0;  // in ms
unsigned int updateInterval = 500; // can be dynamic (WinUpdateMS)
unsigned int lastFrameTime = 0;    // duration of last draw




    // Wrap the full content into lines that fit the window width.
    // This simplistic approach uses a fixed-width estimation.
    void updateWrappedLines() {
        wrappedLines.clear();

        // Convert content (std::string) to an Arduino String for easier manipulation.
        String textStr = String(content.c_str());
        int charWidth = 6 * config.textsize;
        int maxCharsPerLine = (config.width - 4) / charWidth;
        int len = textStr.length();
        String currentLine = "";
        int i = 0;
        while (i < len) {
            // Get next word (words separated by a space)
            int spaceIndex = textStr.indexOf(' ', i);
            if (spaceIndex == -1) spaceIndex = len;
            String word = textStr.substring(i, spaceIndex);
            if (currentLine.length() + word.length() + (currentLine.length() > 0 ? 1 : 0) > maxCharsPerLine) {
                // Push current line and start a new one
                wrappedLines.push_back(currentLine);
                currentLine = "";
            }
            if (currentLine.length() > 0) {
                currentLine += " ";
            }
            currentLine += word;
            i = spaceIndex + 1;
        }
        if (currentLine.length() > 0) {
            wrappedLines.push_back(currentLine);
        }
    }
};

// Global window registry (if needed)
std::vector<std::unique_ptr<Window>> windowRegistry;

void registerWindow(std::unique_ptr<Window> win) {
    windowRegistry.push_back(std::move(win));
}

void unregisterWindow(Window* win) {
    windowRegistry.erase(
        std::remove_if(windowRegistry.begin(), windowRegistry.end(),
            [&](const std::unique_ptr<Window>& w) { return w.get() == win; }),
        windowRegistry.end());
}














///***************************************************************************************************************************************************************************************************************************************************************
//operating system defaults for various types of screen setups
//default groupings are groups of default windows to put on the screen in some conditions, saving you time from having to manually add one of each window to the screen [lock screen,app screen, etc]
///***************************************************************************************************************************************************************************************************************************************************************

//NONE NOW, WE NOW USE APPLICATIONS FOR THIS

#endif
