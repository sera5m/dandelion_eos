
#ifndef Micro2D_A_H
#define Micro2D_A_H

class Window;
class WindowManager;
class Canvas;

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <esp32-hal-spi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <esp32-hal-psram.h>
#include <SD.h>

#define SSD1351_CMD_COMMANDLOCK   0xFD
#define SSD1351_CMD_DISPLAYOFF    0xAE
#define SSD1351_CMD_MUXRATIO      0xCA
#define SSD1351_CMD_SETREMAP      0xA0
#define SSD1351_CMD_STARTLINE     0xA1
#define SSD1351_CMD_DISPLAYOFFSET 0xA2
#define SSD1351_CMD_SETGPIO       0xB5
#define SSD1351_CMD_FUNCTIONSEL   0xAB
#define SSD1351_CMD_NONINVERT     0xA6
#define SSD1351_CMD_CONTRASTABC   0xC1
#define SSD1351_CMD_CLOCKDIV      0xB3
#define SSD1351_CMD_MASTER_CURRENT_CONTROL 0x87
#define SSD1351_CMD_CONTRASTABC   0xC1
#define SSD1351_CMD_ENHANCE       0xB2
#define SSD1351_CMD_DISPLAYON     0xAF

#define BYTES_PER_PIXEL 2  // 16-bit color (RGB565)
#define BATCH_SIZE 64

#define icon_transparency_color 0x5220; //this disgusting brown color will be parsed as clear by the bitmap parser

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 

#define BYTES_PER_PIXEL 2  // RGB565

//adafruit text size nonsense
#define DefaultCharHeight  8
#define DefaultCharWidth  6

//colllllllllllllllllllllllllllorrrrrrrrrrrrrrrrrssssssssssssssssssssssssssccccccccccccoooooooooooooollllllllllloooorrrrrrrrrrsssssccccccccccooooooooooooooolllllllllloooooooorrrrs
// black,white,grey
#define BLACK   0x0000
#define WHITE   0xFFFF
#define Grey 0xDDDD

//regular colors
#define RED     0xF800
#define YELLOW  0xFFE0 
#define GREEN   0x07E0
#define CYAN    0x07FF
#define BLUE    0x001F
#define MAGENTA 0xF81F
#define PURPLE  0x780F

//other color defaults
#define PEACH   0xFD20 
// Forward declarations to resolve circular dependencies
/*
class Window;
class Canvas;
class WindowManager;*/

//structs
extern uint16_t tcol_background;
 struct TextChunk {
    bool isTag;
    std::string value;
};
typedef struct {
    uint8_t x = 0, y = 0, width = 64, height = 64;
    bool AutoAlignment = false, WrapText = true;
    int TextSize = 1;
    bool borderless = false;

    // Pointers to color values
    const uint16_t BorderColor;
    const uint16_t BgColor;
    const uint16_t WinTextColor;

    uint16_t UpdateTickRate = 500;
} WindowCfg;

enum class DrawType {
    Text, Line, Pixel, FRect, Rect, RFRect, RRect,
    Triangle, FTriangle, FCircle, Circle, Bitmap
};

struct CanvasCfg {
    int x = 0, y = 0, width = 32, height = 32;
    bool borderless=true; 
    bool DrawBG=true;
     uint16_t bgColor = 0x0000, BorderColor = 0xFFFF; 
      Window* parentWindow = nullptr;
      uint16_t UpdateTickRate = 100;
    };


struct TextData {
    int posX, posY;
    char text[64];  // Fixed-size buffer for text
    uint8_t txtsize;
    uint16_t color;
};

struct LineData {
    int posX0, posY0, posX1, posY1;
    uint16_t color;
};

struct PixelData {
    int posX, posY;
    uint16_t color;
};

struct FRectData {
    int posX, posY, w, h;
    uint16_t color;
};

struct RectData {
    int posX, posY, w, h;
    uint16_t color;
};

struct RFRectData {
    int posX, posY, w, h, r;
    uint16_t color;
};

struct RRectData {
    int posX, posY, w, h, r;
    uint16_t color;
};

struct TriangleData {
    int x0, y0, x1, y1, x2, y2;
    uint16_t color;
};

struct FTriangleData {
    int x0, y0, x1, y1, x2, y2;
    uint16_t color;
};

struct FCircleData {
    int posX, posY, r;
    uint16_t color;
};

struct CircleData {
    int posX, posY, r;
    uint16_t color;
};

struct BitmapData {
    int posX, posY;
    const uint16_t* bitmap;
    int w, h;
};

struct PixelDat {  // This should match what you're using in your vector
    int posX, posY;
    uint16_t color;
    uint layer;
};
union DrawCommand {
    TextData text;
    LineData line;
    PixelData pixel;
    FRectData frect;
    RectData rect;
    RFRectData rfrect;
    RRectData rrect;
    TriangleData triangle;
    FTriangleData ftriangle;
    FCircleData fcircle;
    CircleData circle;
    BitmapData bitmap;

};

// 4. Modified DrawableElement structure
struct DrawableElement {
    uint layer;
    DrawType type;
    DrawCommand command;
};

struct TextState {
    uint16_t color;
    uint8_t size;
    int16_t cursorX;
    int16_t cursorY;
    bool underline;
    bool strikethrough;
};





//function definitions



void drawPixel(uint16_t x, uint16_t y, uint16_t color);
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFillRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint16_t color);
void tft_Fillscreen(uint16_t color);
void drawVerticalLine(int x, int y0, int y1, uint16_t color);
void drawHorizontalLine(int y, int x0, int x1, uint16_t color);
void drawOutlineRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void drawRectOutline(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void DrawBitmap(const uint16_t* data, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void DrawBitmapICON_DMA(const uint16_t* data, uint16_t x, uint16_t y,    uint16_t w, uint16_t h, uint8_t cnkSizeICOd,bool islazy, uint16_t BGCol);

static uint16_t read16(File &f);
static uint32_t read32(File &f);

//i can't take it man
void x_listFiles(File dir, uint8_t indent = 0);


void DrawBmpFromSD(const char *path, uint16_t x, uint16_t y);

void screen_reboot();
void writeCommand(uint8_t cmd);
void writeData(uint8_t data);
void screen_startup();
void screen_on();
void screen_off();




void CanvasForceParentUpdate(std::shared_ptr<Window> p);//forward slop dependancy 4 this, do not edit pls



//todo get the canvas made here
// WindowManager singleton



class Window : public std::enable_shared_from_this<Window> {
    public:

    //now private to avoid landmine kicking?

        std::string name;
        WindowCfg cfg;
        std::string content;
        std::vector<std::shared_ptr<Canvas>> canvases;
    
        unsigned int UpdateTickRate = 100;
        bool dirty = false;
        unsigned long lastUpdateTime = 0;
        unsigned int lastFrameTime = 0;
    
        uint16_t win_internal_color_background = 0x0000;
        uint16_t win_internal_color_border = 0xFFFF;
        uint16_t win_internal_color_text = 0xFFFF;
    
        uint8_t win_internal_textsize = 1;
        uint16_t win_internal_width = 16;
        uint16_t win_internal_height = 10;
        uint16_t win_internal_x = 32;
        uint16_t win_internal_y = 32;
    
        bool IsWindowShown = true;
    
        int scrollOffsetX = 0;
        int scrollOffsetY = 0;
        int accumDX = 0, accumDY = 0;
        int ScrollaccumDX = 0, ScrollaccumDY = 0;
        uint64_t lastScrollTime = 0;
    
        static constexpr int scrollLimit = 3;
        static constexpr int scrollPeriod = 100;
    
        Window(const std::string& WindowName, const WindowCfg& windowConfiguration, const std::string& initialContent = "");
        ~Window();
    
        void setWinTextSize(uint8_t t);
        void ForceUpdate(bool UpdateSubComps);
        void ForceUpdateSubComps();
        void ApplyTheme(uint16_t BORDER_COLOR, uint16_t BG_COLOR, uint16_t WIN_TEXT_COLOR);
    
        void SetBgColor(uint16_t newColor);
        void SetBorderColor(uint16_t newColor);
        void ForceBorderState(bool isShown);
        void ResizeWindow(int newW, int newH, bool fUpdate);
        void MoveWindow(int newX, int newY, bool fUpdate);
        void addCanvas(const CanvasCfg& cfg);
        void WindowScroll(int DX, int DY);
        void animateMove(int targetX, int targetY, int steps);
        void HideWindow();
        void ShowWindow();
        void setUpdateMode(bool manualOnly);
        void updateContent(const std::string &newContent);
        void WinDraw();
        void updateWrappedLinesOptimized();
        void drawVisibleLinesOptimized();
    
    private:
        struct TextState {
            uint16_t color;
            uint8_t size;
            int16_t cursorX, cursorY;
            bool underline = false;
            bool strikethrough = false;
        };
    
        using WrappedLine = std::vector<TextChunk>;        std::vector<std::vector<TextChunk>> wrappedLines;


        bool manualUpdateOnly = true;
        uint32_t lastContentUpdate = 0;
    
        std::string Delim_LinBreak = "<n>";
        std::string Delim_Seperator = "<_>";
        std::string Delim_ColorChange = "<setcolor(";
        std::string Delim_PosChange = "<pos(";
        std::string Delim_Sizechange = "<textsize(";
        std::string Delim_Strikethr = "<s>";
        std::string Delim_Underline = "<u>";

         std::vector<TextChunk> tokenize(const std::string &input);
        void wrapTextIntoLines(const std::string& text, std::vector<TextChunk>& currentLine, int maxCharsPerLine);
        void applyTextState(const TextState& state);
        void handleTextTag(const std::string& tag, TextState& state);
        void renderTextLine(const std::string& line, int yPos, TextState initialState);
        void renderTextChunk(const std::string& text, TextState& state);
        int calculateTotalTextWidth();
    };

    void CanvasForceParentUpdate(std::shared_ptr<Window> parent);

    class Canvas {
    public:
        // ctor / dtor
        Canvas(const CanvasCfg& cfg, std::shared_ptr<Window> parent);
        ~Canvas();
    
        // clearing
        void clear();
    
        // drawing primitives
        void AddTextLine(int posX, int posY, const String& text,
                         uint8_t txtsize, uint16_t color, int layer = 0);
    
        void AddLine(int posX0, int posY0, int posX1, int posY1,
                     uint16_t color, int layer = 0);
    
        void AddPixel(int posX, int posY, uint16_t color, int layer = 0);
        void AddFRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0);
        void AddRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0);
        void AddRFRect(int posX, int posY, int w, int h, uint16_t r,
                       uint16_t color, int layer = 0);
        void AddRRect(int posX, int posY, int w, int h, uint16_t r,
                      uint16_t color, int layer = 0);
        void AddTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                         uint16_t x2, uint16_t y2, uint16_t color, int layer = 0);
        void AddFTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint16_t color, int layer = 0);
        void AddFCircle(int posX, int posY, int r, uint16_t color, int layer = 0);
        void AddCircle(int posX, int posY, int r, uint16_t color, int layer = 0);
        void AddBitmap(int posX, int posY, const uint16_t* bitmap,
                       int w, int h, int layer = 0);
    
        // orchestrate full-canvas draw & updates
        void CanvasDraw();
        void CanvasUpdate(bool force);
    
        // clear all draw commands
        void ClearAll();
    
    private:
        // geometry & style
        int    x, y, width, height;
        bool   borderless;
        bool   DrawBG;
        uint16_t bgColor, BorderColor;
    
        // state
        std::weak_ptr<Window> parentWindow;
        std::vector<DrawableElement> drawElements;
        bool   canvasDirty = true;
        unsigned long lastUpdateTime = 0;
        unsigned int UpdateTickRate = 100;

    
        // any private helpers…
    };
    
    struct WindowAndUpdateInterval {
        std::weak_ptr<Window> windowWeakPtr;
        int UpdateTickRate;
    
        WindowAndUpdateInterval(std::shared_ptr<Window> Win) 
            : windowWeakPtr(Win), UpdateTickRate(Win->UpdateTickRate) {}
    
        void updateIfValid() {
            if (auto WinPtr = windowWeakPtr.lock()) {
                WinPtr->WinDraw();
            }
        }
    };
    
    
    class WindowManager {
    public:
        // Singleton access
        static WindowManager& getInstance() {
            static WindowManager instance;
            return instance;
        }
    
        // Public destructor
        ~WindowManager();
        
        // Remove copy/move operations
        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;
        WindowManager(WindowManager&&) = delete;
        WindowManager& operator=(WindowManager&&) = delete;
    
        // Existing methods
        void registerWindow(std::shared_ptr<Window> Win);
        void unregisterWindow(Window* Win);
        void clearAllWindows();
        std::shared_ptr<Window> GetWindowByName(const std::string& WindowName);
        void UpdateAllWindows(bool Force, bool AndSubComps);
        void ApplyThemeAllWindows(uint16_t secondary, uint16_t background, uint16_t primary);
        void notifyUpdateTickRateChange(Window* targetWindow, int newUpdateTickRate);
        
        // Initialize graphics system
        bool initialize(bool graphicsEnabled);
    
    private:
        WindowManager(); // Private constructor
        
        bool AreGraphicsEnabled = false;
        std::vector<WindowAndUpdateInterval> WindowRegistry;
    };




    //extern std::unique_ptr<WindowManager> WinManagerInstance;
void clearScreenEveryXCalls(uint16_t x);






 





#endif