// LILLYPAD_RENDERER.h

#ifndef Micro2D_A_H
#define Micro2D_A_H


#include <memory>
#include "Wiring.h"
#include <vector> 
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

//move this to another file later, PLEASE

#define BATCH_SIZE 64

#define icon_transparency_color 0x5220; //this disgusting brown color will be parsed as clear by the bitmap parser

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 

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
#define MAX_SPAN   ((SCREEN_WIDTH > SCREEN_HEIGHT) ? SCREEN_WIDTH : SCREEN_HEIGHT)
static uint8_t spanBuf[MAX_SPAN * 2];

extern SPIClass spiBus;
extern Adafruit_SSD1351 tft;  /* fucking shit needs to be imported. why is this not treated as a global object from the fucking screen setup*/

constexpr int MIN_WINDOW_WIDTH = 18;
constexpr int MIN_WINDOW_HEIGHT = 12;



void TFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  // Early out if nothing to draw
  if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;

  // Clip width and height if going out of bounds
  if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
  if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;

  uint8_t hi = color >> 8, lo = color & 0xFF;
  uint32_t totalPixels = (uint32_t)w * h;

  // Prepare batch buffer (color repeated)
  static uint8_t buf[BATCH_SIZE * 2];
  for (int i = 0; i < BATCH_SIZE; ++i) {
    buf[2 * i]     = hi;
    buf[2 * i + 1] = lo;
  }

  // --- START TIMER ---
  //unsigned long start = millis();
  
  spiBus.beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));

  // Select OLED (CS low)
  FGPIO_LOW(SPI_CS_OLED);
  FGPIO_LOW(OLED_DC);
  spiBus.write(0x15);  // Set column address
  FGPIO_HIGH(OLED_DC);
  spiBus.write(x);
  spiBus.write(x + w - 1);

  FGPIO_LOW(OLED_DC);
  spiBus.write(0x75);  // Set row address
  FGPIO_HIGH(OLED_DC);
  spiBus.write(y);
  spiBus.write(y + h - 1);

  FGPIO_LOW(OLED_DC);
  spiBus.write(0x5C);  // Write RAM
  FGPIO_HIGH(OLED_DC);

  // Write pixel data in chunks
  uint32_t remaining = totalPixels;
  while (remaining > 0) {
    uint32_t cnt = remaining > BATCH_SIZE ? BATCH_SIZE : remaining;
    spiBus.writeBytes(buf, cnt * 2);
    remaining -= cnt;
  }

  FGPIO_HIGH(SPI_CS_OLED);  // CS high to end transaction
  spiBus.endTransaction();

  // --- END TIMER ---
  //unsigned long end = millis();Serial.print("TFillRect took ");Serial.print(end - start);Serial.println(" ms");
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

// ——— DIRECTORY LISTING ———
void listFiles(File dir, uint8_t indent = 0) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    for (uint8_t i=0; i<indent; i++) Serial.print(' ');
    Serial.print(entry.isDirectory() ? "[DIR] " : "      ");
    Serial.println(entry.name());
    if (entry.isDirectory()) {
      listFiles(entry, indent + 2);
    }
    entry.close();
  }
}

// ——— DRAW BMP FROM SD ———
// example path = "/img/cat.bmp", x/y = top-left on 128×128
void DrawBmpFromSD(const char *path, uint16_t x, uint16_t y) {
  File bmp = SD.open(path, FILE_READ);
  if (!bmp) {
    Serial.printf("file read error %s\n", path);
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
    Serial.printf("Only 16 or 24-bit BMP supported, got %d\n", depth);
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
      Serial.printf("First row drawn at Y=%d from file offset %lu\n", drawY, pos);
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
 

void set_orientation(uint8_t rotation);

// Function for adjusting SPI speed and debugging
void set_spi_speed(uint32_t frequency) {
  Serial.print("SPI speed set to: ");
  Serial.print(frequency / 1000000);  // Print in MHz
  Serial.println(" MHz");
}

// Add a function to toggle the reset pin properly
void screen_reboot() {
  digitalWrite(14, LOW);   // Set RST low to reset
  delay(50);                    // Hold low for a bit
  digitalWrite(14, HIGH);  // Set RST high to bring out of reset
  delay(150);                   // Delay for stability
}


void screen_startup() {
  //not having serial cfg here because it needs to be setup in the main code anyway
   //SPI.begin(SPI_SCK, MISO, MOSI_PIN, CS_PIN);
   //SPI.beginTransaction(SPISettings(SPI_FREQUENCY_OLED, MSBFIRST, SPI_MODE0));  // Set SPI speed to 40 MHz
  tft.begin();
  tft.fillScreen(BLACK);
  tft.setCursor(32, 64);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("booting..... ");

    //Serial.println("screen rdy");
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

void set_orientation(uint8_t rotation) {
   tft.setRotation(rotation); 
   //Serial.print("Screen rotation set to ");
   Serial.println(rotation);
}





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
    uint8_t x = 0, y = 0, width = 64, height = 64;
    bool AutoAlignment=0,WrapText=1; // Align text centrally or not,WrapText text
    int TextSize=1; //default text size inside this Window
    bool borderless=false;
    uint16_t BorderColor, bgColor, WinTextColor; // Colors -add defaults
    uint16_t UpdateTickRate=500;//update every how many ms?
}; 

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
};

uint16_t randomColor() {
  static uint16_t seed = 0xACE1;  // Any non-zero seed
  seed ^= seed << 7;
  seed ^= seed >> 9;
  seed ^= seed << 8;
  return seed & 0xFFFF;  // Return 16-bit color
}


void CanvasForceParentUpdate(std::shared_ptr<Window> p);//forward slop dependancy 4 this, do not edit pls



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

// Initialize the static Window manager structure
static std::unique_ptr<WindowManager> WinManagerInstance;


//end forward dependencieslastUpdateTime



//****************************************************************************************************************************************

//canvas draw logic
//canvases are pannels that you can draw various things in, and they'll keep it from spilling off. great for doing stuff like Windows

//******************************************************************************************************************************************************
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

Canvas(const CanvasCfg& cfg, std::shared_ptr<Window> parent): x(cfg.x), y(cfg.y), width(cfg.width), height(cfg.height), borderless(cfg.borderless),DrawBG(cfg.DrawBG), bgColor(cfg.bgColor), BorderColor(cfg.BorderColor), parentWindow(parent) {}

    // Clear the canvas area
    void clear() { //todo: modify for backgroundless 
        // Fill the canvas area with the background color to clear it
        // Note: if you need clipping, ensure tft supports it or implement it yourself,dear user.
        if(DrawBG) {
        TFillRect(x, y, width, height, bgColor);
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
    void AddTextLine(int posX, int posY, const String& text, uint8_t txtsize, 
                    uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.type = DrawType::Text;
        strncpy(element.command.text.text, text.c_str(), sizeof(element.command.text.text) - 1);
        element.command.text.text[sizeof(element.command.text.text) - 1] = '\0';
        element.command.text.posX = posX;
        element.command.text.posY = posY;
        element.command.text.txtsize = txtsize;
        element.command.text.color = color;
        drawElements.push_back(element);
        canvasDirty = true;
    }//i made it more convoluted



//draw shapes **********************************************************************************
//********************these are shapes the user should be calling to draw in canvas,each one should be set to it's own layer-non automatically set as of now, cope. plus you get more controll**********************************************

    void AddLine(int posX0, int posY0, int posX1, int posY1, 
                uint16_t color, int layer = 0) {
        DrawableElement element;
        element.layer = layer;
        element.type = DrawType::Line;
        element.command.line = {posX0, posY0, posX1, posY1, color};
        drawElements.push_back(element);
        canvasDirty = true;
    }
  //adafruit has optimized line draWing and normal line draWing, for an angular one it's drawLine,
  // for a perfectly vertical or horizontal line it's  drawFastVLine or drawFastHLine. fortunately they take simular args, so here i've switched between them







//draw multiple pixels on multiple layers
// Implementation of all drawing methods
void AddPixel(int posX, int posY, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::Pixel;
    element.command.pixel = {posX, posY, color};
    drawElements.push_back(element);
    canvasDirty = true;
}



void AddFRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::FRect;
    element.command.frect = {posX, posY, w, h, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddRect(int posX, int posY, int w, int h, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::Rect;
    element.command.rect = {posX, posY, w, h, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddRFRect(int posX, int posY, int w, int h, uint16_t r, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::RFRect;
    element.command.rfrect = {posX, posY, w, h, r, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddRRect(int posX, int posY, int w, int h, uint16_t r, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::RRect;
    element.command.rrect = {posX, posY, w, h, r, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, 
                uint16_t x2, uint16_t y2, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::Triangle;
    element.command.triangle = {x0, y0, x1, y1, x2, y2, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddFTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::FTriangle;
    element.command.ftriangle = {x0, y0, x1, y1, x2, y2, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddFCircle(int posX, int posY, int r, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::FCircle;
    element.command.fcircle = {posX, posY, r, color};
    drawElements.push_back(element);
    canvasDirty = true;
}

void AddCircle(int posX, int posY, int r, uint16_t color, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::Circle;
    element.command.circle = {posX, posY, r, color};
    drawElements.push_back(element);
    canvasDirty = true;
}



void AddBitmap(int posX, int posY, const uint16_t* bitmap, int w, int h, int layer = 0) {
    DrawableElement element;
    element.layer = layer;
    element.type = DrawType::Bitmap;
    element.command.bitmap = {posX, posY, bitmap, w, h};
    drawElements.push_back(element);
    canvasDirty = true;
}





//back to main draw logic for canvas here



    // Draws the canvas: first draws the canvas background (and border if not borderless),
    // then sorts and draws the drawable elements by layer.
    void CanvasDraw() {
    if(DrawBG) TFillRect(x, y, width, height, bgColor);
    if(!borderless) tft.drawRect(x, y, width, height, BorderColor);

    std::sort(drawElements.begin(), drawElements.end(), 
        [](const DrawableElement& a, const DrawableElement& b) {
            return a.layer < b.layer;
        });

    for(const auto& elem : drawElements) {
        switch(elem.type) {
            case DrawType::Text:
                tft.setTextColor(elem.command.text.color);
                tft.setTextSize(elem.command.text.txtsize);
                tft.setCursor(x + elem.command.text.posX, y + elem.command.text.posY);
                tft.print(elem.command.text.text);
                break;
            
            case DrawType::Line: {
    const auto& l = elem.command.line;  // define l here
//NEED TO REPLACE TODO FIX MEEE
    if(l.posX0 == l.posX1) {
        tft.drawFastVLine(x + l.posX0, y + std::min(l.posY0, l.posY1), std::abs(l.posY1 - l.posY0), l.color);
    } else if(l.posY0 == l.posY1) {
        tft.drawFastHLine(x + std::min(l.posX0, l.posX1), y + l.posY0, std::abs(l.posX1 - l.posX0), l.color);
    } else {
        drawLine(x + l.posX0, y + l.posY0, x + l.posX1, y + l.posY1, l.color);
    }
    break;
}

            
            case DrawType::Pixel:
                tft.drawPixel(x + elem.command.pixel.posX, 
                             y + elem.command.pixel.posY, 
                             elem.command.pixel.color);
                break;
                

                
            case DrawType::FRect:
                TFillRect(x + elem.command.frect.posX,
                            y + elem.command.frect.posY,
                            elem.command.frect.w,
                            elem.command.frect.h,
                            elem.command.frect.color);
                break;
                
            case DrawType::Rect:
                tft.drawRect(x + elem.command.rect.posX,
                            y + elem.command.rect.posY,
                            elem.command.rect.w,
                            elem.command.rect.h,
                            elem.command.rect.color);
                break;
                
            case DrawType::RFRect:
                tft.fillRoundRect(x + elem.command.rfrect.posX,
                                y + elem.command.rfrect.posY,
                                elem.command.rfrect.w,
                                elem.command.rfrect.h,
                                elem.command.rfrect.r,
                                elem.command.rfrect.color);
                break;
                
            case DrawType::RRect:
                tft.drawRoundRect(x + elem.command.rrect.posX,
                                y + elem.command.rrect.posY,
                                elem.command.rrect.w,
                                elem.command.rrect.h,
                                elem.command.rrect.r,
                                elem.command.rrect.color);
                break;
                
            case DrawType::Triangle:
                tft.drawTriangle(x + elem.command.triangle.x0,
                                y + elem.command.triangle.y0,
                                x + elem.command.triangle.x1,
                                y + elem.command.triangle.y1,
                                x + elem.command.triangle.x2,
                                y + elem.command.triangle.y2,
                                elem.command.triangle.color);
                break;
                
            case DrawType::FTriangle:
                tft.fillTriangle(x + elem.command.ftriangle.x0,
                                y + elem.command.ftriangle.y0,
                                x + elem.command.ftriangle.x1,
                                y + elem.command.ftriangle.y1,
                                x + elem.command.ftriangle.x2,
                                y + elem.command.ftriangle.y2,
                                elem.command.ftriangle.color);
                break;
                
            case DrawType::FCircle:
                tft.fillCircle(x + elem.command.fcircle.posX,
                               y + elem.command.fcircle.posY,
                               elem.command.fcircle.r,
                               elem.command.fcircle.color);
                break;
                
            case DrawType::Circle:
                tft.drawCircle(x + elem.command.circle.posX,
                              y + elem.command.circle.posY,
                              elem.command.circle.r,
                              elem.command.circle.color);
                break;
                
            case DrawType::Bitmap: {
                const auto& bmp = elem.command.bitmap;
                tft.drawRGBBitmap(x + bmp.posX, y + bmp.posY, bmp.bitmap, bmp.w, bmp.h);
                break;
            }
                
            default:
                // Handle unexpected types (should never happen)
                break;
        }  // This was the missing closing brace for the switch
    }
}
    void CanvasUpdate(bool force) { //only updates if dirty,chexks as to not waste frames
        int now=millis();//now less long
        if (canvasDirty || (millis() - lastUpdateTime >= UpdateTickRate)|| force) {
            CanvasDraw();
            lastUpdateTime = millis();
            canvasDirty = false;
        }
    }

void ClearAll() {
    //for(auto& elem : drawElements) {

    //}
    drawElements.clear();
    canvasDirty = true;
}

~Canvas() {

    //for(auto& elem : drawElements) { }
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
//config.bgColor = 0x0000; // Black
//config.BorderColor = 0xFFFF; // White
//config.WinTextColor = 0xFFFF; // White

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
    uint16_t WindowBackgroundColor=config.bgColor;
    bool IsWindowShown = true;//windows are shown by default on creation

       int scrollOffsetX=0; //scroll offsets for these Windows //TODO:MOVE THESE TO PRIVATE
       int scrollOffsetY=0;

      int accumDX = 0, accumDY = 0; // member var for the stupid scroll func: todo: move to private, this is awful


    // Constructor
Window(const std::string& WindowName, const WindowCfg& cfg, const std::string& initialContent = "")  : name(WindowName), config(cfg), content(initialContent) {
    //enforce window is at least the minimum size for safety reasons and to not drive me insane
    if (config.width < MIN_WINDOW_WIDTH) config.width = MIN_WINDOW_WIDTH;
    if (config.height < MIN_WINDOW_HEIGHT) config.height = MIN_WINDOW_HEIGHT;
}


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



//why didn't i add this before
void SetBgColor(uint16_t newColor) {
    if(newColor > 0xFFFF) {
        Serial.println("Invalid color value!");
        return;
    }
    WindowBackgroundColor = newColor; //formerly set the struct, now sets the var color. not sure why it crashed?
    dirty = true;
}

    // Set border color
    void SetBorderColor(uint16_t newColor) {
        if (config.BorderColor != newColor) {
            config.BorderColor = newColor;
            dirty = true;
        }
    }

    // Toggle border visibility
    void ForceBorderState(bool isShown) {
        if (config.borderless != !isShown) {
            config.borderless = !isShown;
            dirty = true;
        }
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
    //Serial.println("updateContent called");  Serial.print("New content size: "); Serial.println(newContent.size());

    if (content != newContent) {
        //Serial.println("Content differs, updating...");
        content = newContent;
        dirty = true;
        WinDraw(); // <- this is where it probably crashes
    } else {
        //Serial.println("Content is same, skipping");
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
    //validation  mrowl

    
          unsigned long startTime = millis();//framerate counter, time counter start
        // Clear the Window area
        //Serial.println("bg color is");

        //Serial.println(WindowBackgroundColor);
        TFillRect(config.x, config.y, config.width, config.height, WindowBackgroundColor); //fill the Window with the background color

        //Serial.println("filled background");  if (!config.borderless) Serial.print("drew border"); 
        //draw background

         if(config.borderless){
            drawOutlineRect(config.x, config.y, config.x+config.width, config.y+config.height, config.BorderColor);
          }

        //draw lines


        //TFillRect(config.x, config.y, config.width, config.height, config.BorderColor); //draw the rectangle outline
        
        // Set text properties
        tft.setTextColor(config.WinTextColor);
        tft.setTextSize(config.TextSize);

        // Update wrapped lines from full content
        updateWrappedLines(); 
       // Serial.println("wrapped lines updated");


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
            Serial.println("draw canvas");
            canvas->CanvasUpdate(true);  // Update/draw the canvas. todo: do a better method than force
        }//end if valid statement
    }//end canvas iterator


  }//end of is window shown?
    //else { //Serial.println("Window hidden (IsWindowShown = false)");  } //you never know. probably should remove later but... debug print statements are important
    //Serial.println("void draw done");

 }//end void draw


 void HideWindow() {
  IsWindowShown = false; //note:this automatically will stop void winupdate from updating even when called: because we have the flag in THERE
  TFillRect(config.x, config.y, config.width, config.height,0x0000);//fill the area with black, default color

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

const int linesPerFrame = 3; //drawing 4 and over lines of text at once occasionally results in data corruption in the vram of the display, this is a countermeasure where we draw lines as chunks
int currentDrawingPosition = 0; // Track our position in the wrappedLines vector
bool isPartialDraw = false; // Are we in the middle of a partial draw?

// ================== drawVisibleLines ==================
__attribute__((optimize("O2"))) // heehoo magic
void drawVisibleLines() {
  if (IsWindowShown) {
    int baseTextSize = config.TextSize;
    uint16_t currentColor = config.WinTextColor;
    int currentTextSize = baseTextSize;
    uint16_t originalWinTexColor = config.WinTextColor;
    int charHeight = 8 * currentTextSize;
    int charWidth = 6 * currentTextSize;
    int startLine = scrollOffsetY / charHeight;
    int visibleLines = (config.height - 4) / charHeight;
    int y = config.y + 2 - (scrollOffsetY % charHeight);

    //TFillRect(config.x, config.y, config.width, config.height, WindowBackgroundColor);
    //square should be aware of window size in the future but why bother. todo make it aware of textsize by basic mult
    //i don't think this should even be here as this is a string drawing function

    for (int i = startLine; i < wrappedLines.size() && i < startLine + visibleLines; i++) {
      const std::string &line = wrappedLines[i];
      int startChar = max(0, scrollOffsetX / charWidth);
      int visibleChars = (config.width - 4) / charWidth;

      tft.setTextSize(currentTextSize);
      tft.setTextColor(currentColor);

      int cursorX = config.x + 2;
      int cursorY = y;
      tft.setCursor(cursorX, cursorY);

      std::string segment;
      int charsProcessed = 0;
      size_t pos = startChar;

      while (pos < line.length() && charsProcessed < visibleChars) {
        if (line[pos] == '<') {
          size_t tagEnd = line.find('>', pos);
          if (tagEnd != std::string::npos) {
            // Flush any accumulated text before handling the tag
            if (!segment.empty()) {
              tft.print(segment.c_str());
              segment.clear();
            }

            std::string_view tag(&line[pos], tagEnd - pos + 1);
            if (tag == Delim_LinBreak) { // Line break: stop processing further
              break;
            }

            else if (tag.substr(0, 5) == Delim_PosChange) {
              size_t comma = tag.find(',');
              if (comma != std::string::npos) {
                int newX = std::stoi(std::string(tag.substr(5, comma - 5)));
                int newY = std::stoi(std::string(tag.substr(comma + 1, tag.find(')') - comma - 1)));
                cursorX = config.x + newX;
                cursorY = config.y + newY;
                tft.setCursor(cursorX, cursorY);
              }
            }
             else if (tag.substr(0, Delim_Sizechange.length()) ==Delim_Sizechange){ //damn thing wasn't here before
              size_t start = tag.find('(');
              if (start != std::string::npos) {
                start++; // move past '('
                size_t endParen = tag.find(')', start);
                if (endParen != std::string::npos && endParen > start) {
                  std::string_view colorStr = tag.substr(start, endParen - start);
                  uint8_t windownewtextsize = std::stoul(std::string(colorStr), nullptr, 8); //bad copied code
                  tft.setTextSize(windownewtextsize);
                } else {
                  printf("Error: Malformed size tag");
                }
              }
            
             }
            else if (tag.substr(0, Delim_ColorChange.length()) == Delim_ColorChange) {
              size_t start = tag.find('(');
              if (start != std::string::npos) {
                start++; // move past '('
                size_t endParen = tag.find(')', start);
                if (endParen != std::string::npos && endParen > start) {
                  std::string_view colorStr = tag.substr(start, endParen - start);
                  uint32_t rawColor = std::stoul(std::string(colorStr), nullptr, 16); //shouldn't be 32 bit, colors are 16 bit wtf fix this
                  uint16_t color16 = rawColor & 0xFFFF;
                  tft.setTextColor(color16);
                } else {
                  printf("Error: Malformed color tag");
                }
              }
            }
            else if (tag == Delim_Underline) {
              int lineY = cursorY + charHeight - 1;
              tft.drawFastHLine(cursorX, lineY, (visibleChars - charsProcessed) * charWidth, currentColor);
            }
            else if (tag == Delim_Strikethr) {
              int lineY = cursorY + (charHeight / 2);
              tft.drawFastHLine(cursorX, lineY, (visibleChars - charsProcessed) * charWidth, currentColor);
            }
            else if (tag == Delim_Seperator) { // Print a space for separator
              tft.print(" ");
              charsProcessed++;
              cursorX += charWidth;
            }
            else {
              // If unrecognized, optionally add the tag text to the segment
              segment.append(tag);
            }
            pos = tagEnd + 1;
          }
          else { // Malformed tag; treat as normal char
            segment.push_back(line[pos]);
            pos++;
            charsProcessed++;
            cursorX += charWidth;
          }
        }
        else { // Normal character
          segment.push_back(line[pos]);
          pos++;
          charsProcessed++;
          cursorX += charWidth;
        }
        if (cursorX > config.x + config.width - 2)
          break;
      }
      // Flush any remaining text in the segment for the line
      if (!segment.empty()) {
        tft.print(segment.c_str());
      }
      y += charHeight;
      // Reset text size & color for next line
      currentTextSize = baseTextSize;
      tft.setTextSize(currentTextSize);
      tft.setTextColor(originalWinTexColor);
    }
  }
}


void updateWrappedLines() {


  //Serial.println(esp_timer_get_time());
  //Serial.println("init line wrap calc");



    wrappedLines.clear();
    int baseCharWidth = 6 * config.TextSize;
    int maxCharsPerLine = (config.width - 4) / baseCharWidth;

    // Reserve some capacity for the current line to avoid repeated reallocations.
    std::string currentLine;
    currentLine.reserve(maxCharsPerLine);

    const size_t len = content.length();
    size_t pos = 0;
    int currentLineLength = 0;

    while (pos < len) {
        if (content[pos] == '<') {
            // Find the closing '>' manually
            size_t end = pos;
            while (end < len && content[end] != '>') { 
                ++end;
            }
            if (end < len && content[end] == '>') {
                // Instead of constructing a new string using substr,
                // we use a temporary string_view (or minimal string copy) if available.
                // Here, we do a simple copy; in embedded cases you might avoid even that.
                std::string tag(content, pos, end - pos + 1);
                
                if (tag == Delim_LinBreak) { // Hard line break
                    wrappedLines.push_back(currentLine);
                    currentLine.clear();
                    currentLineLength = 0;
                }
                else if (tag == Delim_Seperator) { // Separator tag
                    if (currentLineLength + 1 <= maxCharsPerLine) {
                        currentLine.push_back(' ');
                        currentLineLength++;
                    }


                    else {
                        wrappedLines.push_back(currentLine);
                        currentLine = " ";
                        currentLineLength = 1;
                    }
                }
                else if (tag == Delim_Underline || tag == Delim_Strikethr) {
                    // Simply append the tag for formatting purposes.
                    currentLine.append(tag);
                }
                else {
                    currentLine.append(tag);
                }
                pos = end + 1;
            }
            else { // Malformed tag: no closing '>'
                currentLine.push_back(content[pos]);
                currentLineLength++;
                pos++;
            }
        }
        else { // Normal character
            char c = content[pos];
            if (c == ' ') { // Word wrap point
                if (currentLineLength + 1 > maxCharsPerLine) {
                    wrappedLines.push_back(currentLine);
                    currentLine.clear();
                    currentLineLength = 0;
                }
                else {
                    currentLine.push_back(' ');
                    currentLineLength++;
                }
            }
            else { // Regular character
                if (currentLineLength >= maxCharsPerLine) {
                    wrappedLines.push_back(currentLine);
                    currentLine.clear();
                    currentLineLength = 0;
                }
                currentLine.push_back(c);
                currentLineLength++;
            }
            pos++;
        }
    }

    if (!currentLine.empty()) {
        wrappedLines.push_back(currentLine);
    }

    // Draw the visible lines after processing
          //Serial.println(esp_timer_get_time());
  //Serial.println("final line wrap");
    drawVisibleLines();

}



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
        TFillRect(Win->config.x, Win->config.y, Win->config.width, Win->config.height, 0x0000);
    }

/*
void Callback2WinManager_Window_deleted (){//todo: say which one
Serial.print("something deleted a Window.\n"); }
*/

 // Clear all Windows
    void clearAllWindows() {
        for (auto& entry : WindowRegistry) {
            if (auto winPtr = entry.windowWeakPtr.lock()) {
                TFillRect(winPtr->config.x, winPtr->config.y, winPtr->config.width, winPtr->config.height, 0x0000);
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
