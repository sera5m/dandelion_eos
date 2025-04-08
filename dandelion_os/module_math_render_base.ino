// module_math_render_base.h

#ifndef module_math_render_base_H
#define module_math_render_base_H

// Screen dimensions
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
#define PEACH   0xFD20  // Default window color


//Todo here later
//include more fonts https://github.com/olikraus/u8glib/wiki/fontsize


#include <Adafruit_NeoPixel.h> //





// Define an adjustable SPI frequency
// by default spi is 8mhz. i have adjusted the adafruit library to use a higher frequency
#define SPI_FREQUENCY 40000000  // 40 MHz 

// Define the TFT display object with the SSD1351 chipset
Adafruit_SSD1351 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN); 


void set_orientation(uint8_t rotation);

// Function for adjusting SPI speed and debugging
void set_spi_speed(uint32_t frequency) {
  Serial.print("SPI speed set to: ");
  Serial.print(frequency / 1000000);  // Print in MHz
  Serial.println(" MHz");
}

// Add a function to toggle the reset pin properly
void screen_reset() {
  digitalWrite(RST_PIN, LOW);   // Set RST low to reset
  delay(50);                    // Hold low for a bit
  digitalWrite(RST_PIN, HIGH);  // Set RST high to bring out of reset
  delay(150);                   // Delay for stability
}


void screen_startup() {
  //not having serial cfg here because it needs to be setup in the main code anyway
   //SPI.begin(SCLK_PIN, MISO, MOSI_PIN, CS_PIN);
   //SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));  // Set SPI speed to 40 MHz
  tft.begin();
  tft.fillScreen(BLACK);
  tft.setCursor(32, 64);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("booting..... ");

    //Serial.println("screen rdy");
}

void screen_on() {
   digitalWrite(RST_PIN, HIGH);  // Assuming RST_PIN controls screen power
  // Serial.println("Screen turned on");
}

void screen_off() {
   digitalWrite(RST_PIN, LOW);  // Assuming RST_PIN controls screen power
   //Serial.println("Screen turned off");
}

void set_orientation(uint8_t rotation) {
   tft.setRotation(rotation); 
   //Serial.print("Screen rotation set to ");
   Serial.println(rotation);
}

#endif
