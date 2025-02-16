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
//end forward dependencies


//****************************************************************************************************************************************

//canvas draw logic
//canvases are pannels that you can draw various things in, and they'll keep it from spilling off. great for doing stuff like windows

//******************************************************************************************************************************************************


// A DrawableElement represents any element (text or shape) drawn on the canvas.
struct DrawableElement {
    int layer;  // Lower layer drawn first, higher layers drawn on top
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
    std::vector<DrawableElement> drawElements;
    
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
    void addTextLine(int posX, int posY, const String &text, uint16_t textColor, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=, text]() { //forces string copy so it's still available for lambda after this goes outta scope
            tft.setTextColor(textColor);
            // Set the cursor relative to canvas origin.
            tft.setCursor(x + posX, y + posY);
            tft.print(text);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
    // Adds a basic shape: example for drawing a rectangle.
    // (You can add similar helper functions for circles, lines, triangles, etc.)
    void addRectangle(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.drawFunc = [=, text]() { //forces string copy so it's still available for lambda after this goes outta scope
            // Draw a filled rectangle at the given position relative to the canvas.
            tft.fillRect(x + posX, y + posY, w, h, color);
        };
        drawElements.push_back(element);
        canvasDirty = true;
    }
    
    // Draws the canvas: first draws the canvas background (and border if not borderless),
    // then sorts and draws the drawable elements by layer.
    void draw() {
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
        tft.fillRect(config.x, config.y, config.width, config.height, config.bgColor);
        if (!config.borderless)
            tft.drawRect(config.x, config.y, config.width, config.height, config.borderColor);

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



#endif
