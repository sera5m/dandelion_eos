


#include <memory>
#include "Wiring.h"
#include <vector> 
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <esp32-hal-spi.h>
//move this to another file later, PLEASE
#include "Micro2D_A.h"
#include <SD.h>

/*
using WrappedLine = std::vector<TextChunk>;

std::vector<std::vector<TextChunk>> wrappedLines;
//oh god, please don't tell me these things all share a buffer. that's a landmine.
*/
#define MAX_SPAN   ((SCREEN_WIDTH > SCREEN_HEIGHT) ? SCREEN_WIDTH : SCREEN_HEIGHT)
static uint8_t spanBuf[MAX_SPAN * 2];

extern SPIClass spiBus;
extern Adafruit_SSD1351 tft;  /* fucking shit needs to be imported. why is this not treated as a global object from the fucking screen setup*/

constexpr int MIN_WINDOW_WIDTH = 18;
constexpr int MIN_WINDOW_HEIGHT = 12;

uint16_t ScreenBackgroundColor=0x0000; //defined in the settings tab, you jerk off. this is just the default. you will have to use this in main and load pallette in main
// Global/static buffer (safer than stack)



// PSRAM framebuffer

#include <esp32-hal-psram.h>

// Screen dimensions

// Global framebuffer in PSRAM
uint16_t* framebuffer = NULL; // PSRAM framebuffer
SPIClass* hspi = NULL; // SPI instance

/*
bool initScreenBuffer() {
    if (!psramFound()) {
        Serial.println("No PSRAM - falling back to slow mode");
        return false;
    }

    framebuffer = (uint16_t*)ps_malloc(SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
    if (!framebuffer) {
        Serial.println("PSRAM alloc failed!");
        return false;
    }
    
    memset(framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL);
    return true;
}

void pushToScreen() {
    if (!framebuffer) return;

    tft.startWrite();
    tft.setAddrWindow(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
    tft.writePixels(framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
    tft.endWrite();
}
*/


/*
void flushFramebuffer() {
    if (!framebuffer) return;

    // 1. Set address window (full screen)
    tft.startWrite();
    tft.setAddrWindow(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
    
    // 2. DMA-accelerated transfer
    #ifdef ESP32
    hspi->beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));
    digitalWrite(SPI_CS_OLED, LOW);
    digitalWrite(OLED_DC, HIGH); // Data mode
    
    // Queue SPI DMA transfer (non-blocking)
    spiWriteBytes(hspi->bus(), (uint8_t*)framebuffer, SCREEN_WIDTH*SCREEN_HEIGHT*2);
    
    digitalWrite(SPI_CS_OLED, HIGH);
    hspi->endTransaction();
    #else
    // Fallback for non-ESP32
    tft.writePixels(framebuffer, SCREEN_WIDTH*SCREEN_HEIGHT);
    #endif
    
    tft.endWrite();
}*/

// Optimized pixel drawing
void drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    framebuffer[y * SCREEN_WIDTH + x] = color;
}

// Optimized rectangle fill
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // Clip to screen bounds
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
    
    for (uint16_t i = y; i < y + h; i++) {
        for (uint16_t j = x; j < x + w; j++) {
            framebuffer[i * SCREEN_WIDTH + j] = color;
        }
    }
}
void TFillRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint16_t color){
tft.drawRect(x,y,w,h,color);
}

void tft_Fillscreen(uint16_t color){
TFillRect(0,0,128,128,color);

}

// Draw a vertical line at column x, from row y0 to y1 (inclusive)
void drawVerticalLine(int x, int y0, int y1, uint16_t color) {
  // Clip
  x  = constrain(x, 0, SCREEN_WIDTH - 1);
  y0 = constrain(y0, 0, SCREEN_HEIGHT - 1);
  y1 = constrain(y1, 0, SCREEN_HEIGHT - 1);

  if (y1 < y0) std::swap(y0, y1);
  int span = y1 - y0 + 1;

  // Prepare span buffer
  uint8_t hi = color >> 8, lo = color & 0xFF;
  for (int i = 0; i < span; ++i) {
    spanBuf[2*i]   = hi;
    spanBuf[2*i+1] = lo;
  }

  // SPI transaction
  spiBus.beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));
  FGPIO_LOW(SPI_CS_OLED);

  // Column: x→x, Row: y0→y1
  FGPIO_LOW(OLED_DC); spiBus.write(0x15);
  FGPIO_HIGH(OLED_DC); spiBus.write(x); spiBus.write(x);
  FGPIO_LOW(OLED_DC); spiBus.write(0x75);
  FGPIO_HIGH(OLED_DC); spiBus.write(y0); spiBus.write(y1);

  // Write RAM
  FGPIO_LOW(OLED_DC); spiBus.write(0x5C);
  FGPIO_HIGH(OLED_DC);
  spiBus.writeBytes(spanBuf, span * 2);

  FGPIO_HIGH(SPI_CS_OLED);
  spiBus.endTransaction();
}


// Draw a horizontal line at row y, from column x0 to x1 (inclusive)
void drawHorizontalLine(int y, int x0, int x1, uint16_t color) {
  // Clip
  y  = constrain(y, 0, SCREEN_HEIGHT - 1);
  x0 = constrain(x0, 0, SCREEN_WIDTH  - 1);
  x1 = constrain(x1, 0, SCREEN_WIDTH  - 1);

  if (x1 < x0) std::swap(x0, x1);
  int span = x1 - x0 + 1;

  // Prepare span buffer
  uint8_t hi = color >> 8, lo = color & 0xFF;
  for (int i = 0; i < span; ++i) {
    spanBuf[2*i]   = hi;
    spanBuf[2*i+1] = lo;
  }

  // SPI transaction
  spiBus.beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));
  FGPIO_LOW(SPI_CS_OLED);

  // Column: x0→x1, Row: y→y
  FGPIO_LOW(OLED_DC); spiBus.write(0x15);
  FGPIO_HIGH(OLED_DC); spiBus.write(x0); spiBus.write(x1);
  FGPIO_LOW(OLED_DC); spiBus.write(0x75);
  FGPIO_HIGH(OLED_DC); spiBus.write(y);  spiBus.write(y);

  // Write RAM
  FGPIO_LOW(OLED_DC); spiBus.write(0x5C);
  FGPIO_HIGH(OLED_DC);
  spiBus.writeBytes(spanBuf, span * 2);

  FGPIO_HIGH(SPI_CS_OLED);
  spiBus.endTransaction();
}

void drawOutlineRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
  // Clip swap to ensure top-left to bottom-right logic
  if (x1 < x0) std::swap(x0, x1);
  if (y1 < y0) std::swap(y0, y1);

  drawHorizontalLine(y0, x0, x1, color); // top
  drawHorizontalLine(y1, x0, x1, color); // bottom
  drawVerticalLine(x0, y0, y1, color);   // left
  drawVerticalLine(x1, y0, y1, color);   // right
}


void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  while (x0 != x1 || y0 != y1) {
    TFillRect(x0, y0, 1, 1, color);
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }

  // Draw final point
  TFillRect(x1, y1, 1, 1, color);
}



void drawRectOutline(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  if (w == 0 || h == 0) return;

  uint8_t hi = color >> 8, lo = color & 0xFF;

  spiBus.beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));
  FGPIO_LOW(SPI_CS_OLED);

  // === Top horizontal line ===
  FGPIO_LOW(OLED_DC);
  spiBus.write(0x15); // Set column
  FGPIO_HIGH(OLED_DC);
  spiBus.write(x);
  spiBus.write(x + w - 1);

  FGPIO_LOW(OLED_DC);
  spiBus.write(0x75); // Set row
  FGPIO_HIGH(OLED_DC);
  spiBus.write(y);
  spiBus.write(y);

  FGPIO_LOW(OLED_DC);
  spiBus.write(0x5C); // Write RAM
  FGPIO_HIGH(OLED_DC);

  for (int i = 0; i < w; ++i) {
    spiBus.write(hi);
    spiBus.write(lo);
  }

  // === Bottom horizontal line ===
  FGPIO_LOW(OLED_DC);
  spiBus.write(0x15);
  FGPIO_HIGH(OLED_DC);
  spiBus.write(x);
  spiBus.write(x + w - 1);

  FGPIO_LOW(OLED_DC);
  spiBus.write(0x75);
  FGPIO_HIGH(OLED_DC);
  spiBus.write(y + h - 1);
  spiBus.write(y + h - 1);

  FGPIO_LOW(OLED_DC);
  spiBus.write(0x5C);
  FGPIO_HIGH(OLED_DC);

  for (int i = 0; i < w; ++i) {
    spiBus.write(hi);
    spiBus.write(lo);
  }

  // === Left and Right vertical lines ===
  for (int side = 0; side < 2; ++side) {
    uint8_t xline = (side == 0) ? x : x + w - 1;

    FGPIO_LOW(OLED_DC);
    spiBus.write(0x15);
    FGPIO_HIGH(OLED_DC);
    spiBus.write(xline);
    spiBus.write(xline);

    FGPIO_LOW(OLED_DC);
    spiBus.write(0x75);
    FGPIO_HIGH(OLED_DC);
    spiBus.write(y);
    spiBus.write(y + h - 1);

    FGPIO_LOW(OLED_DC);
    spiBus.write(0x5C);
    FGPIO_HIGH(OLED_DC);

    for (int i = 0; i < h; ++i) {
      spiBus.write(hi);
      spiBus.write(lo);
    }
  }

  FGPIO_HIGH(SPI_CS_OLED);
  spiBus.endTransaction();
}

void DrawBitmap(const uint16_t* data, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  if (x >= 128 || y >= 128) return;
  uint16_t x_end = x + w - 1;
  uint16_t y_end = y + h - 1;
  if (x_end >= 128) x_end = 127;
  if (y_end >= 128) y_end = 127;

  spiBus.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  FGPIO_LOW(SPI_CS_OLED);

  // Set column address
  FGPIO_LOW(OLED_DC); spiBus.write(0x15); FGPIO_HIGH(OLED_DC);
  spiBus.write(x); spiBus.write(x_end);

  // Set row address
  FGPIO_LOW(OLED_DC); spiBus.write(0x75); FGPIO_HIGH(OLED_DC);
  spiBus.write(y); spiBus.write(y_end);

  // Write RAM command
  FGPIO_LOW(OLED_DC); spiBus.write(0x5C); FGPIO_HIGH(OLED_DC);

  // Send pixels
  
for (uint32_t i = 0; i < (w * h); i++) {
    uint16_t color = data[i];
    //if (color != icon_transparency_color) {
        spiBus.write(static_cast<uint8_t>(color >> 8));    // send high byte 
        spiBus.write(static_cast<uint8_t>(color & 0xFF));  // send low byte
   // }
}


  FGPIO_HIGH(SPI_CS_OLED);
  spiBus.endTransaction();
}


//chunked run

void DrawBitmapICON_DMA(const uint16_t* data,
                       uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h, uint8_t cnkSizeICOd,
                       bool islazy, uint16_t BGCol) {
    // Clip to display bounds
    if (x >= 128 || y >= 128 || cnkSizeICOd > w) return;

    if (x + w > 128) w = 128 - x;
    if (y + h > 128) h = 128 - y;

    // Dynamic buffer (automatically manages memory)
    std::vector<uint16_t> colBuf(cnkSizeICOd * h);  // Resizes as needed

    spiBus.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

    for (uint16_t block = 0; block < w; block += cnkSizeICOd) {
        uint16_t blockSize = (w - block >= cnkSizeICOd) ? cnkSizeICOd : (w - block);
        uint16_t px_start = x + block;
        uint16_t px_end = px_start + blockSize - 1;

        // Fill buffer in COLUMN-major order
        for (uint16_t colOff = 0; colOff < blockSize; colOff++) {
            for (uint16_t row = 0; row < h; row++) {
                uint16_t color = data[row * w + (block + colOff)];
#ifdef ICON_TRANSPARENT_COLOR
                colBuf[colOff * h + row] = (color == ICON_TRANSPARENT_COLOR) ? BGCol : color;
#else
                colBuf[colOff * h + row] = color;
#endif
            }
        }

        // Set column/row address window
        FGPIO_LOW(SPI_CS_OLED);
        FGPIO_LOW(OLED_DC); spiBus.write(0x15); FGPIO_HIGH(OLED_DC);
        spiBus.write(px_start);
        spiBus.write(px_end);

        FGPIO_LOW(OLED_DC); spiBus.write(0x75); FGPIO_HIGH(OLED_DC);
        spiBus.write(y);
        spiBus.write(y + h - 1);

        // Write RAM command + burst transfer
        FGPIO_LOW(OLED_DC); spiBus.write(0x5C); FGPIO_HIGH(OLED_DC);
        spiBus.writeBytes(reinterpret_cast<uint8_t*>(colBuf.data()), blockSize * h * 2);
        FGPIO_HIGH(SPI_CS_OLED);
    }

    spiBus.endTransaction();
}



//stream bmp from file to display. WARNING: UNSURE IF IT WORKS. WHAT i want to do is pull from the sd and directly dump it's data to the oled. sd is flash, and oled is ram. both memory types
//todo: support more than 128x
// ——— FILE-READ HELPERS ———
static uint16_t read16(File &f) {
  uint8_t lo = f.read(), hi = f.read();
  return (hi << 8) | lo;
}
static uint32_t read32(File &f) {
  uint32_t b0 = f.read(), b1 = f.read(), b2 = f.read(), b3 = f.read();
  return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}



// ——— DRAW BMP FROM SD ———
// example path = "/img/cat.bmp", x/y = top-left on 128×128
void DrawBmpFromSD(const char *path, uint16_t x, uint16_t y) {
  File bmp = SD.open(path, FILE_READ);
  if (!bmp) {
    //Serial.printf("file read error %s\n", path);
    return;
  }

  // ——— HEADER ———
  if (bmp.read() != 'B' || bmp.read() != 'M') {
    Serial.println("Not a BMP");
    bmp.close(); 
    return;
  }

  read32(bmp);            // file size
  read32(bmp);            // creator bytes
  uint32_t imgOffset = read32(bmp);
  uint32_t headerSize = read32(bmp);
  int32_t  bmpW       = read32(bmp);
  int32_t  bmpH       = read32(bmp);
  uint16_t planes     = read16(bmp);
  uint16_t depth      = read16(bmp);
  uint32_t comp       = read32(bmp);

  // support BI_RGB (0) or BI_BITFIELDS (3)
  bool bitfields = false;
  uint32_t redMask = 0, greenMask = 0, blueMask = 0;
  if (comp == 3) {
    bitfields = true;
    redMask   = read32(bmp);
    greenMask = read32(bmp);
    blueMask  = read32(bmp);
  }

  if (planes != 1 || (comp != 0 && comp != 3)) {
    Serial.println("Unsupported BMP (must be uncompressed or bitfields)");
    bmp.close();
    return;
  }

  if (depth != 16 && depth != 24) {
    //Serial.printf("Only 16 or 24-bit BMP supported, got %d\n", depth);
    bmp.close();
    return;
  }

  // row size (padded to 4 bytes)
  uint32_t rowSize = ((bmpW * depth / 8) + 3) & ~3;

  if (x >= 128 || y >= 128) {
    bmp.close();
    return;
  }

  uint16_t x2 = min((int32_t)x + bmpW - 1, (int32_t)127);
  uint16_t y2 = min((int32_t)y + bmpH - 1, (int32_t)127);
  uint16_t drawW = x2 - x + 1;

  // ——— DRAW ———
  for (int row = 0; row < bmpH; row++) {
    uint32_t pos = imgOffset + (uint32_t)(bmpH - 1 - row) * rowSize;
    bmp.seek(pos);
    uint16_t drawY = y2 - row;  // flip bits vertically becasue bmp is bottom up and the screen goes top down

    if (drawY >= 128) continue;

    spiBus.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    FGPIO_LOW(SPI_CS_OLED);

    FGPIO_LOW(OLED_DC); spiBus.write(0x15); FGPIO_HIGH(OLED_DC); // column addr
    spiBus.write(x); spiBus.write(x2);
    FGPIO_LOW(OLED_DC); spiBus.write(0x75); FGPIO_HIGH(OLED_DC); // row addr
    spiBus.write(drawY); spiBus.write(drawY);
    FGPIO_LOW(OLED_DC); spiBus.write(0x5C); FGPIO_HIGH(OLED_DC); // write RAM

    if (depth == 24) {
      for (int col = 0; col < drawW; col++) {
        uint8_t b = bmp.read(), g = bmp.read(), r = bmp.read();
        uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        spiBus.write(c >> 8);
        spiBus.write(c & 0xFF);
      }
    } else {  // 16-bit
      for (int col = 0; col < drawW; col++) {
        uint16_t raw = read16(bmp);
        raw = (raw << 8) | (raw >> 8);  // BMP is usually little-endian
        spiBus.write(raw >> 8);
        spiBus.write(raw & 0xFF);
      }
    }

    FGPIO_HIGH(SPI_CS_OLED);
    spiBus.endTransaction();

    if (row == 0) {
      //Serial.printf("First row drawn at Y=%d from file offset %lu\n", drawY, pos);
    }
  }

  bmp.close();
}




//#include "AT_SSD1351.ino" //note: arduino ide joins ino files, meaning we'd need it to convert to a .h and .cpp file so if we leave this like it is 
//todo:

//little alert icons
//jpeg+png+bmp


//was gonna name this orchid renderer but some guy made a render engine already named that and it was cool tbh


//this is the Window handler for dandelionn.
//this module handles Window creation and draw calls. version X, now with performance enhancements. :3



// Pin definitions for spi





//Todo here later
//include more fonts https://github.com/olikraus/u8glib/wiki/fontsize


//#include <Adafruit_NeoPixel.h> //





// Define an adjustable SPI frequency
// by default spi is 8mhz. i have adjusted the adafruit library to use a higher frequency for more fps!
 

void set_orientation(uint8_t rotation);//what is this

// Function for adjusting SPI speed and debugging


// Add a function to toggle the reset pin properly
void screen_reboot() {
  digitalWrite(14, LOW);   // Set RST low to reset
  delay(50);                    // Hold low for a bit
  digitalWrite(14, HIGH);  // Set RST high to bring out of reset
  delay(150);                   // Delay for stability
}

void writeCommand(uint8_t cmd) {
  digitalWrite(OLED_DC, LOW);  // Command mode
  digitalWrite(SPI_CS_OLED, LOW);
  SPI.transfer(cmd);
  digitalWrite(SPI_CS_OLED, HIGH);
}

void writeData(uint8_t data) {
  digitalWrite(OLED_DC, HIGH);  // Data mode
  digitalWrite(SPI_CS_OLED, LOW);
  SPI.transfer(data);
  digitalWrite(SPI_CS_OLED, HIGH);
}

 void screen_startup() {
  tft.begin();
  tft.fillScreen(BLACK);

  tft.setCursor(32, 64);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("booting..... ");
}



void screen_on() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  delay(150);
}


void screen_off() {
   digitalWrite(OLED_RST, LOW);  // Assuming RST_PIN controls screen power
   //Serial.println("Screen turned off");
}






//forward declare all possible dependencies/child of Window before we use them
/*
class Canvas;  // forward declaration 
class Window; //do i have to define the Window class?
class WindowManager; //only make one of these on os start
//include the struct dependencies of children

*///userspace graphics config import do not delete this or the whole system will fuck up
 bool AreGraphicsEnabled = true;//userconfig
 bool IsWindowHandlerAlive = false;//do not touch this in user code ever, for the Window manager code only. "hey guys what if we could break the entire goddamn Window manager on a whim!!!" "get the fuck outta my office"

//define it here, it's extern in main idk




//end forward dependencieslastUpdateTime



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



Window::Window(const std::string& WindowName, const WindowCfg& windowConfiguration, const std::string& initialContent)
    : name(WindowName),
      cfg(windowConfiguration),
      content(initialContent),
      win_internal_textsize(cfg.TextSize),
      win_internal_color_background(cfg.BgColor),
      win_internal_color_border(cfg.BorderColor),
      win_internal_color_text(cfg.WinTextColor),
      UpdateTickRate(cfg.UpdateTickRate)
{
    win_internal_width  = std::max<int>(MIN_WINDOW_WIDTH, cfg.width);
    win_internal_height = std::max<int>(MIN_WINDOW_HEIGHT, cfg.height);
    win_internal_x = cfg.x;
    win_internal_y = cfg.y;
}


    // Destructor: Clean up canvases
Window::~Window() {
  //WinManagerInstance->Callback2WinManager_Window_deleted();  // Custom callback when the Window is deleted-disabled untill i get proper singlet ref
    // Any other custom cleanup tasks
}
void Window::setWinTextSize(uint8_t t){
    win_internal_textsize=t;
}

void Window::ForceUpdate(bool UpdateSubComps) {//todo: toggle to NOT update offscreen canvas comps-this updates em all by force
    dirty = true;
    WinDraw();  // Immediately update Window content now, FUCKING RIGHT NOW 
    if (UpdateSubComps) {
        for (auto& canvas : canvases) {
            // Force update on each canvas (even if off-screen or timer-based)
            if (canvas) canvas->CanvasUpdate(UpdateSubComps); //push canvas update for iterator-this makes sure canvas-> update checks for valid first
        }
    }
}

void Window::ApplyTheme(uint16_t BORDER_COLOR, uint16_t BG_COLOR, uint16_t WIN_TEXT_COLOR) {
    win_internal_color_background = BG_COLOR;
    win_internal_color_border = BORDER_COLOR;
    win_internal_color_text = WIN_TEXT_COLOR;
    ForceUpdate(true);
}


void Window::ForceUpdateSubComps(){ //todo: toggle to NOT update offscreen canvas comps
  for (auto& canvas : canvases) {
            // Force update on each canvas (even if off-screen or timer-based)
            if (canvas) canvas->CanvasUpdate(true); //push canvas update for iterator -this makes sure canvas-> update checks for valid first
        }
}

//change properties===============================================================================================

//why didn't i add this before
void Window::SetBgColor(uint16_t newColor) {
    win_internal_color_background = newColor; //sets internal ref, does not alter configs
    dirty = true;
}

    // Set border color
void Window::SetBorderColor(uint16_t newColor) {
    if (win_internal_color_border != newColor) {
        win_internal_color_border = newColor;
        dirty = true;
    }
}


    // Toggle border visibility
    void Window::ForceBorderState(bool isShown) {
        if (cfg.borderless != !isShown) {
            cfg.borderless = !isShown;
            dirty = true;
        }
    }


//scaling
void Window::ResizeWindow(int newW, int newH,bool fUpdate) {
    if (win_internal_width == newW && win_internal_height == newH)
        return;

    // Erase the _old_ area first:
    TFillRect(win_internal_x, win_internal_y, win_internal_width, win_internal_height, ScreenBackgroundColor);

    // Now update your internals:
    win_internal_width  = newW;
    win_internal_height = newH;

    // And redraw at the new size:
    ForceUpdate(fUpdate);
}



void Window::MoveWindow(int newX, int newY,bool fUpdate) { //mofe from old location to new
    if (win_internal_x == newX && win_internal_y == newY) return; // No change, no need to update
    TFillRect(win_internal_x,win_internal_y,win_internal_width,win_internal_height,ScreenBackgroundColor);//xywh
    win_internal_x = newX;
    win_internal_y = newY;
    
    ForceUpdate(fUpdate); // Force a redraw since the position changed
}

//================================================================================
/*
void setUpdateTickRate(int newRate) {
    UpdateTickRate = newRate;
    WindowManager::getWinManagerInstance().notifyUpdateTickRateChange(this); //change the variable in the update rate
}*/



    // Add a canvas to this Window
    void Window::addCanvas(const CanvasCfg& cfg) {

        if (cfg.parentWindow != this) {
        Serial.print("Error: Canvas parent mismatch in Window: "); Serial.println(name.c_str());
        //reject request, it's on the wrong line
            return;
        }
        //safe,create new canvas+pushback-smart ptr
    auto newCanvas = std::make_shared<Canvas>(cfg, shared_from_this());
    canvases.push_back(newCanvas);
    }



// Global or member variables governming this Window s scrolligng
int ScrollaccumDX = 0, ScrollaccumDY = 0;
uint64_t lastScrollTime = millis(); 
const int scrollLimit = 3; // max scrolls per period
const int scrollPeriod = 100; // in ms

int Window::calculateTotalTextWidth() {
    int charWidth = 6 * win_internal_textsize; // Assuming fixed-width font
    int maxWidth = 0;
    for (const auto& line : wrappedLines) {
        int lineWidth = line.size() * charWidth;
        if (lineWidth > maxWidth) {
            maxWidth = lineWidth;
        }
    }
    return maxWidth;
}

//new scroll code, now supporting directions
void Window::WindowScroll(int DX, int DY) { //changes in directions
    // accumulate scroll deltas
accumDX += DX;
accumDY += DY;

    
    uint64_t now = millis();
    if ( (now - lastScrollTime) >= scrollPeriod || abs(accumDX) >= scrollLimit || abs(accumDY) >= scrollLimit) { 
      //if you scroll a bunch of fucking times in a time period we will group scrolls together before applying them,preventing extra writes to vram. untested as of 3/17/2025

        // update scroll offsets
        scrollOffsetX += accumDX;
        scrollOffsetY += accumDY;
     int totalTextHeight = wrappedLines.size() * (8 * win_internal_textsize);  // Total height of all the wrapped text
    int maxOffsetY = (totalTextHeight > win_internal_height - 4) ? totalTextHeight - (win_internal_height - 4) : 0;
    scrollOffsetY = std::max(0, std::min(scrollOffsetY, maxOffsetY));
    
    // Clamp horizontal scroll offset (if needed)
    int totalTextWidth = calculateTotalTextWidth();  // todo: add better logic for this so we can skip ahead in the text
int maxOffsetX = (totalTextWidth > win_internal_width - 4) ? totalTextWidth - (win_internal_width - 4) : 0;

    scrollOffsetX = std::max(0, std::min(scrollOffsetX, maxOffsetX));
        
        // Reset accumulators & update last time stamp BEFORE checking limits
        lastScrollTime = now; //we just scrolled
        int prevOffsetX = scrollOffsetX;
        int prevOffsetY = scrollOffsetY;
        accumDX = 0; 
        accumDY = 0;

        // Notify components to update only if scroll changed
        if (scrollOffsetX != prevOffsetX || scrollOffsetY != prevOffsetY) {
            
            (true); // push an update RIGHT FUCKING NOW
          
        }

    }
}


void Window::animateMove(int targetX, int targetY, int steps) { //move the Window but try to animate it
    int stepX = (targetX - win_internal_x) / steps;
    int stepY = (targetY - win_internal_y) / steps;
    
    for (int i = 0; i < steps; i++) {
        win_internal_x += stepX;
        win_internal_y += stepY;
        ForceUpdate(false);
        delay(45); // Small delay to make animation visible
    }
    
    // Ensure final position is exact
    win_internal_x = targetX;
    win_internal_y = targetY;
    ForceUpdate(true);
}



 void Window::HideWindow() {
  IsWindowShown = false; //note:this automatically will stop void winupdate from updating even when called: because we have the flag in THERE
  tft.fillRect(win_internal_x, win_internal_y, win_internal_width, win_internal_height,ScreenBackgroundColor);//fill the area with background color to hide it

        // Optionally, instruct sub-elements (Canvases) to hide themselves.-needs a fix later
        // for (auto& canvas : Canvases) { if (canvas) canvas->Hide(); }
    }//end hide win

 void Window::ShowWindow() {
  IsWindowShown = true;
  ForceUpdate(true);
        // Optionally, instruct all sub-elements (Canvases) to show themselves.
    }//end show window

 void Window::setUpdateMode(bool manualOnly) {
        manualUpdateOnly = manualOnly;
        Serial.printf("[Config] Update mode set to %s\n",  manualOnly ? "MANUAL" : "AUTO");
    }


void Window::updateContent(const std::string &newContent) {
    if (content == newContent) return;
    content = newContent;
    dirty = true;
}

std::vector<TextChunk> Window::tokenize(const std::string &input) {
    std::vector<TextChunk> chunks;
    size_t pos = 0, len = input.size();

    while (pos < len) {
        if (input[pos] == '<') {
            // Find end of tag
            size_t end = input.find('>', pos);
            if (end == std::string::npos) {
                // Malformed tag: treat '<' as normal char
                chunks.push_back({false, input.substr(pos, 1)});
                pos++;
            } else {
                // Valid tag: include '<' and '>'
                chunks.push_back({true, input.substr(pos, end - pos + 1)});
                pos = end + 1;
            }
        } else {
            // Find next tag start or end of string
            size_t next = input.find('<', pos);
            if (next == std::string::npos) next = len;
            chunks.push_back({false, input.substr(pos, next - pos)});
            pos = next;
        }
    }
    return chunks;
}


void Window::WinDraw(/*bool force*/) {
    if (!IsWindowShown/*|| !dirty*/) return;

    // 1) Clear the window area
    tft.fillRect(win_internal_x, win_internal_y, win_internal_width, win_internal_height,win_internal_color_background);


    // 2) Tokenize the content
    auto chunks = tokenize(content);

    // 3) Setup initial TextState
    struct TextState {
      uint16_t color;
      uint8_t size;
      int16_t x, y;
    } state {
      win_internal_color_text,
      win_internal_textsize,
      int16_t(win_internal_x + 2),
      int16_t(win_internal_y + 2)
    };

    tft.setTextColor(state.color);
    tft.setTextSize(state.size);
    tft.setCursor(state.x, state.y);

    // 4) Render each chunk
    for (auto &chunk : chunks) {
        if (chunk.isTag) {
            // apply tag
            if (chunk.value.starts_with("<setcolor(")) {
                //Serial.printf("color tag");

                auto hex = chunk.value.substr(10, chunk.value.find(')')-10);
                state.color = strtol(hex.c_str(), nullptr, 16);
                tft.setTextColor(state.color);
                
            }
            else if (chunk.value.starts_with("<textsize(")) {
                auto s = chunk.value.substr(10, chunk.value.find(')')-10);
                state.size = atoi(s.c_str());
                tft.setTextSize(state.size);
                //Serial.printf("text size");
            }
            else if (chunk.value == "<n>") {
                //Serial.printf("newline tag");
                // newline: move cursor to next line
                state.y += DefaultCharHeight * state.size;
                state.x = win_internal_x + 2;
                tft.setCursor(state.x, state.y);
            }
            // handle other tags (<pos>, <_<>, underline, etc.)…
        }
        else {
            // plain text
            tft.print(chunk.value.c_str());
            //Serial.printf("no tags applied");
            // advance X cursor manually if you need exact control:
            // state.x += chunk.value.length() * DefaultCharWidth * state.size;
            // tft.setCursor(state.x, state.y);
        }
    }

    dirty = false;
}



//private:
/*
struct TextChunk {
  bool isTag;
  std::string value;
};*/
//using WrappedLine = std::vector<TextChunk>; std::vector<WrappedLine> wrappedLines;

    bool manualUpdateOnly = true; // Default to manual-only mode
    uint32_t lastContentUpdate = 0;
//defaults for delimiters for orderlyness
/*
std::string Delim_LinBreak ="<n>";
std::string Delim_Seperator ="<_>";
//properties
//must be in format of <desire(value)>
std::string Delim_ColorChange = "<setcolor(";
std::string Delim_PosChange = "<pos("; 
std::string Delim_Sizechange = "<textsize(";
//properties of special text
std::string Delim_Strikethr = "<s>";        // Italic text 
std::string Delim_Underline = "<u>";  // Underline text 
*/
// ================== Text Rendering Engine ==================



    void Window::applyTextState(const TextState& state) {
        tft.setTextColor(state.color);
        tft.setTextSize(state.size);
        tft.setCursor(state.cursorX, state.cursorY);
    }

    void Window::handleTextTag(const std::string& tag, TextState& state) {
        if (tag == Delim_LinBreak) {
            return; // Handled during line wrapping
        }
        else if (tag == Delim_Underline) {
            state.underline = true;
        }
        else if (tag == Delim_Strikethr) {
            state.strikethrough = true;
        }
        else if (tag == Delim_Seperator) {
            // Space is handled during rendering
        }
        else if (tag.starts_with(Delim_ColorChange)) {
            size_t start = Delim_ColorChange.size();
            size_t end = tag.find(')', start);
            if (end != std::string::npos) {
                uint16_t color = std::stoul(tag.substr(start, end - start), nullptr, 16);
                state.color = color;
            }
        }
        else if (tag.starts_with(Delim_Sizechange)) {
            size_t start = Delim_Sizechange.size();
            size_t end = tag.find(')', start);
            if (end != std::string::npos) {
                uint8_t size = std::stoul(tag.substr(start, end - start));
                state.size = size;
            }
        }
        else if (tag.starts_with(Delim_PosChange)) {
            size_t start = Delim_PosChange.size();
            size_t end = tag.find(')', start);
            if (end != std::string::npos) {
                size_t comma = tag.find(',', start);
                if (comma != std::string::npos) {
                    int x = std::stoi(tag.substr(start, comma - start));
                    int y = std::stoi(tag.substr(comma + 1, end - comma - 1));
                    state.cursorX = win_internal_x + x;
                    state.cursorY = win_internal_y + y;
                }
            }
        }
    }

    void Window::renderTextLine(const std::string& line, int yPos, TextState initialState) {
        
        TextState currentState = initialState;
        applyTextState(currentState);

        size_t pos = 0;
        std::string textSegment;
        textSegment.reserve(32); // Pre-allocate to reduce allocations

        while (pos < line.size()) {
            if (line[pos] == '<') {
                // Flush any accumulated text
                if (!textSegment.empty()) {
                    tft.print(textSegment.c_str());
                    textSegment.clear();
                }

                // Find tag end
                size_t tagEnd = line.find('>', pos);
                if (tagEnd == std::string::npos) {
                    // Malformed tag, skip
                    pos++;
                    continue;
                }

                std::string tag = line.substr(pos, tagEnd - pos + 1);
                handleTextTag(tag, currentState);
                applyTextState(currentState);

                // Handle special rendering for formatting tags
                if (tag == Delim_Underline || tag == Delim_Strikethr) {
                    int lineY = currentState.cursorY;
                    if (tag == Delim_Underline) {
                        lineY += DefaultCharHeight * currentState.size - 1;
                    } else { // Strikethrough
                        lineY += (DefaultCharHeight * currentState.size) / 2;
                    }
                    tft.drawFastHLine(currentState.cursorX, lineY, 
                                    tft.width() - currentState.cursorX, 
                                    currentState.color);
                }

                pos = tagEnd + 1;
            } else {
                textSegment += line[pos++];
            }
        }

        // Flush remaining text
        if (!textSegment.empty()) {
            tft.print(textSegment.c_str());
        }
        
    }

void Window::updateWrappedLinesOptimized() {
    wrappedLines.clear();
    if (content.empty()) return;

    const int maxCharsPerLine = (win_internal_width - 4) / (DefaultCharWidth * win_internal_textsize);

    WrappedLine currentLine;
    std::string currentText;

    size_t pos = 0;
    while (pos < content.size()) {
        if (content[pos] == '<') {
            // Flush any buffered text
            if (!currentText.empty()) {
                wrapTextIntoLines(currentText, currentLine, maxCharsPerLine);
                currentText.clear();
            }

            // Extract tag as a chunk
            size_t tagEnd = content.find('>', pos);
            if (tagEnd == std::string::npos) {
                // Invalid, treat as text
                currentText += content[pos++];
                continue;
            }

            std::string tag = content.substr(pos, tagEnd - pos + 1);

if (tag == Delim_LinBreak) {
    if (!currentLine.empty()) {
        wrappedLines.push_back(currentLine);
        currentLine.clear();
    } else {
        // <-- THIS FIX
        wrappedLines.push_back(WrappedLine{});
    }
}


            pos = tagEnd + 1;
        } else {
            // Normal text, build word
            size_t wordEnd = content.find_first_of(" <", pos);
            if (wordEnd == std::string::npos) wordEnd = content.size();

            currentText += content.substr(pos, wordEnd - pos);
            pos = wordEnd;
        }
    }

    // Flush trailing text
    if (!currentText.empty()) {
        wrapTextIntoLines(currentText, currentLine, maxCharsPerLine);
    }

    if (!currentLine.empty()) {
        wrappedLines.push_back(currentLine);
    }
}


   void Window::wrapTextIntoLines(const std::string& text, WrappedLine& currentLine, int maxCharsPerLine) {
    size_t start = 0;
    while (start < text.size()) {
        size_t nextSpace = text.find(' ', start);
        if (nextSpace == std::string::npos) nextSpace = text.size();

        std::string word = text.substr(start, nextSpace - start);
        int currentLen = 0;

        for (const auto& chunk : currentLine) {
            if (!chunk.isTag) currentLen += chunk.value.size();
        }

        if (currentLen + word.size() > maxCharsPerLine) {
            if (!currentLine.empty()) {
                wrappedLines.push_back(currentLine);
                currentLine.clear();
            }
        }

        currentLine.push_back({false, word});

        if (nextSpace != text.size()) {
            currentLine.push_back({false, " "});
        }

        start = (nextSpace == text.size()) ? nextSpace : nextSpace + 1;
    }
}
void Window::drawVisibleLinesOptimized() {
    if (!IsWindowShown) return;

    const int charHeight = DefaultCharHeight * win_internal_textsize;
    const int startLine = scrollOffsetY / charHeight;
    const int visibleLines = (cfg.height - 4) / charHeight;
    const int endLine = std::min(startLine + visibleLines, (int)wrappedLines.size());

    tft.fillRect(win_internal_x, win_internal_y, cfg.width, cfg.height, 0x0000 /*win_internal_color_background*/);

    TextState state;
    state.color = win_internal_color_text;
    state.size = win_internal_textsize;
    state.cursorX = win_internal_x + 2;
    state.cursorY = win_internal_y + 2 - (scrollOffsetY % charHeight);
    state.underline = false;
    state.strikethrough = false;

    for (int i = startLine; i < endLine; i++) {
        for (const auto& chunk : wrappedLines[i]) {
            if (chunk.isTag) {
                handleTextTag(chunk.value, state);
            } else {
                renderTextChunk(chunk.value, state);
            }
        }
        state.cursorY += charHeight;
        state.cursorX = win_internal_x + 2;

        // Reset for next line
        state.color = win_internal_color_text;
        state.size = win_internal_textsize;
        state.underline = false;
        state.strikethrough = false;
        applyTextState(state);
    }
}

void Window::renderTextChunk(const std::string& text, TextState& state) {
    tft.setCursor(state.cursorX, state.cursorY);
    //tft.setcolor
    //set text size
    tft.print(text.c_str());
int16_t x = tft.getCursorX();
int16_t y = tft.getCursorY();
x += text.length() * DefaultCharWidth * state.size;
tft.setCursor(x, y);
}



 //end Window obj


//shitty workaround for canvas to force the parent to update
void CanvasForceParentUpdate(std::shared_ptr<Window> p){
  p->ForceUpdate(false); //do not update subcomps to avoid infinite loop. this should say false. DO NOT EDIT THIS CODE
}









//*************************************************************************************
//                        Window manager
//handles Windows and updating. just create a background osproscess with this in it.
//do not create more than one object in main, i've got some code to reject creation of one already exists
//cpp file

WindowManager::WindowManager() {
    IsWindowHandlerAlive = true;
    Serial.print("WindowManager created.\n");
}

WindowManager::~WindowManager() {
    clearAllWindows();
    WinManagerInstance = nullptr;
    IsWindowHandlerAlive = false;
    Serial.print("WindowManager destroyed.\n");
    tft_Fillscreen(ScreenBackgroundColor);
    tft.print("graphics system disabled");
}

WindowManager* WindowManager::getWinManagerInstance() {
    if (!AreGraphicsEnabled) {
        Serial.print("Error: Graphics not enabled. WindowManager will not start.");
        return nullptr;
    }
    if (!WinManagerInstance) {
        WinManagerInstance = std::make_unique<WindowManager>();
    }
    return WinManagerInstance.get();
}

void WindowManager::registerWindow(std::shared_ptr<Window> Win) {
    WindowRegistry.emplace_back(Win);
}

void WindowManager::unregisterWindow(Window* Win) {
    if (!Win) return;

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

    TFillRect(Win->cfg.x, Win->cfg.y, Win->win_internal_width, Win->win_internal_height, ScreenBackgroundColor);
}

void WindowManager::clearAllWindows() {
    for (auto& entry : WindowRegistry) {
        if (auto winPtr = entry.windowWeakPtr.lock()) {
            TFillRect(winPtr->cfg.x, winPtr->cfg.y, winPtr->cfg.width, winPtr->cfg.height, ScreenBackgroundColor);
        }
    }
    WindowRegistry.clear();
    tft_Fillscreen(ScreenBackgroundColor);
}

std::shared_ptr<Window> WindowManager::GetWindowByName(const std::string& WindowName) {
    for (auto& entry : WindowRegistry) {
        if (auto winPtr = entry.windowWeakPtr.lock()) {
            if (winPtr->name == WindowName) {
                return winPtr;
            }
        }
    }
    return nullptr;
}

void WindowManager::UpdateAllWindows(bool Force, bool AndSubComps) {
    unsigned long currentTime = millis();
    for (auto it = WindowRegistry.begin(); it != WindowRegistry.end(); ) {
        if (auto winPtr = it->windowWeakPtr.lock()) {
            if (Force || (currentTime - winPtr->lastUpdateTime >= winPtr->UpdateTickRate)) {
                if (winPtr->dirty || Force) {
                    winPtr->WinDraw();
                    if (AndSubComps) winPtr->ForceUpdateSubComps();
                    winPtr->lastUpdateTime = currentTime;
                    winPtr->dirty = false;
                }
            }
            ++it;
        } else {
            it = WindowRegistry.erase(it);
        }
    }
}

void WindowManager::ApplyThemeAllWindows(uint16_t secondary, uint16_t background, uint16_t primary) {
    for (auto it = WindowRegistry.begin(); it != WindowRegistry.end(); ) {
        if (auto winPtr = it->windowWeakPtr.lock()) {
            winPtr->ApplyTheme(secondary, background, primary);
            ++it;
        } else {
            it = WindowRegistry.erase(it);
        }
    }
    UpdateAllWindows(true, true);
}

void WindowManager::notifyUpdateTickRateChange(Window* targetWindow, int newUpdateTickRate) {
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

void WindowManager::selfDestructWinManager() {
    WinManagerInstance = nullptr;
    clearAllWindows();
    tft.setCursor(0, 64);
    tft_Fillscreen(ScreenBackgroundColor);
    tft.print("graphics disabled");
    Serial.print("shutting down all the fucking graphics. hope you have something to restart it later. try not to call anything when it's shut down.");
}

//void WindowManager::Callback2WinManager_Window_deleted(){   /*nothing yet, but callbacks are useful*/};

//tips: to get Win by name use "Window* myWindow = WindowManager.getWindowByName("Window1");" w/ the name :)

//to del a Win, WindowManager.unregisterWindow(myWindow);
//to wipe em all use WindowManager.clearAllWindows();-should work and not leak memory





//****************************************************************************************************************************************

//canvas draw logic
//canvases are pannels that you can draw various things in, and they'll keep it from spilling off. great for doing stuff like Windows

//******************************************************************************************************************************************************


// Canvas class definition
Canvas::Canvas(const CanvasCfg& cfg, std::shared_ptr<Window> parent)
  : x(cfg.x), y(cfg.y),
    width(cfg.width), height(cfg.height),
    borderless(cfg.borderless),
    DrawBG(cfg.DrawBG),
    bgColor(cfg.bgColor),
    BorderColor(cfg.BorderColor),
    parentWindow(parent),
    UpdateTickRate(cfg.UpdateTickRate)
{
    clear();
}

Canvas::~Canvas() {
    // nothing special for now
}

void Canvas::clear() {
    if (DrawBG) {
        TFillRect(x, y, width, height, bgColor);
        canvasDirty = true;
    } else if (auto p = parentWindow.lock()) {
        CanvasForceParentUpdate(p);
    }
}

void Canvas::AddTextLine(int posX, int posY, const String& text,
                         uint8_t txtsize, uint16_t color, int layer)
{
    DrawableElement e;
    e.layer = layer;
    e.type  = DrawType::Text;
    strncpy(e.command.text.text, text.c_str(),
            sizeof(e.command.text.text) - 1);
    e.command.text.posX   = posX;
    e.command.text.posY   = posY;
    e.command.text.txtsize= txtsize;
    e.command.text.color  = color;
    drawElements.push_back(e);
    canvasDirty = true;
}

void Canvas::AddLine(int posX0, int posY0, int posX1, int posY1,
                     uint16_t color, int layer)
{
    DrawableElement e;
    e.layer = layer;
    e.type  = DrawType::Line;
    e.command.line = { posX0, posY0, posX1, posY1, color };
    drawElements.push_back(e);
    canvasDirty = true;
}

void Canvas::AddPixel(int posX, int posY, uint16_t color, int layer) {
    DrawableElement e;
    e.layer = layer;
    e.type  = DrawType::Pixel;
    e.command.pixel = { posX, posY, color };
    drawElements.push_back(e);
    canvasDirty = true;
}

void Canvas::AddFRect(int posX, int posY, int w, int h,
                      uint16_t color, int layer) {
    DrawableElement e; e.layer = layer; e.type = DrawType::FRect;
    e.command.frect = { posX, posY, w, h, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddRect(int posX, int posY, int w, int h,
                     uint16_t color, int layer) {
    DrawableElement e; e.layer = layer; e.type = DrawType::Rect;
    e.command.rect = { posX, posY, w, h, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddRFRect(int posX, int posY, int w, int h, uint16_t r,
                       uint16_t color, int layer) {
    DrawableElement e; e.layer = layer; e.type = DrawType::RFRect;
    e.command.rfrect = { posX, posY, w, h, r, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddRRect(int posX, int posY, int w, int h, uint16_t r,
                      uint16_t color, int layer) {
    DrawableElement e; e.layer = layer; e.type = DrawType::RRect;
    e.command.rrect = { posX, posY, w, h, r, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddTriangle(uint16_t x0, uint16_t y0, uint16_t x1,
                         uint16_t y1, uint16_t x2, uint16_t y2,
                         uint16_t color, int layer)
{
    DrawableElement e; e.layer = layer; e.type = DrawType::Triangle;
    e.command.triangle = { x0, y0, x1, y1, x2, y2, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddFTriangle(uint16_t x0, uint16_t y0, uint16_t x1,
                          uint16_t y1, uint16_t x2, uint16_t y2,
                          uint16_t color, int layer)
{
    DrawableElement e; e.layer = layer; e.type = DrawType::FTriangle;
    e.command.ftriangle = { x0, y0, x1, y1, x2, y2, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddFCircle(int posX, int posY, int r,
                        uint16_t color, int layer)
{
    DrawableElement e; e.layer = layer; e.type = DrawType::FCircle;
    e.command.fcircle = { posX, posY, r, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddCircle(int posX, int posY, int r,
                       uint16_t color, int layer)
{
    DrawableElement e; e.layer = layer; e.type = DrawType::Circle;
    e.command.circle = { posX, posY, r, color };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::AddBitmap(int posX, int posY, const uint16_t* bitmap,
                       int w, int h, int layer)
{
    DrawableElement e; e.layer = layer; e.type = DrawType::Bitmap;
    e.command.bitmap = { posX, posY, bitmap, w, h };
    drawElements.push_back(e); canvasDirty = true;
}

void Canvas::CanvasDraw() {
    if (DrawBG) TFillRect(x, y, width, height, bgColor);
    if (!borderless) tft.drawRect(x, y, width, height, BorderColor);

    std::sort(drawElements.begin(), drawElements.end(),
              [](auto const &a, auto const &b){ return a.layer < b.layer; });

    for (auto const &e : drawElements) {
        switch (e.type) {
            // uncomment & fill in your drawing code for each DrawType...
            default: break;
        }
    }
}

void Canvas::CanvasUpdate(bool force) {
    auto now = millis();
    if (canvasDirty || (now - lastUpdateTime >= UpdateTickRate) || force) {
        CanvasDraw();
        lastUpdateTime = now;
        canvasDirty = false;
    }
}

void Canvas::ClearAll() {
    drawElements.clear();
    canvasDirty = true;
}

std::unique_ptr<WindowManager> WinManagerInstance;

