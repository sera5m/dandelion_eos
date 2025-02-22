

//common rf frequencies

//2-fsk
//two frequencies to represent 0 and 1
//low power


//4fsk
//2-fsk but with 4 channels but more complex decoding












/*
  RadioLib CC1101 Settings Example

  This example shows how to change all the properties of RF69 radio.
  RadioLib currently supports the following settings:
  - pins (SPI slave select, digital IO 0, digital IO 1)
  - carrier frequency
  - bit rate
  - receiver bandwidth
  - allowed frequency deviation
  - output power during transmission
  - sync word

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#cc1101

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/


//settings



  // carrier frequency:                   434.0 MHz
  // bit rate:                            32.0 kbps
  // frequency deviation:                 60.0 kHz
  // Rx bandwidth:                        250.0 kHz
  // output power:                        7 dBm
  // preamble length:                     32 bits



// CC1101 has the following connections:
// CS pin:     16 
// GDO0 pin:  15 
// RST pin:   unused
// GDO2 pin:   4 
//the rest is just set up with spi setup!


  //CC1101 radio1 = new Module(10, 15, RADIOLIB_NC, 4); //cs,gd0,rst,gdo2
  //to my understanding you can support multiple radio modules if you define em to diff pins.



// or detect the pinout automatically using RadioBoards
// https://github.com/radiolib-org/RadioBoards
/*
#define RADIO_BOARD_AUTO
#include <RadioBoards.h>
Radio radio3 = new RadioModule();
*/




//add recieve transmit functionality here later.  i'll do it after i can actually save packets to ram and/or microsd





















//examples of setting runtime for radiolib

/*
  // carrier frequency:                   434.0 MHz
  // bit rate:                            32.0 kbps
  // frequency deviation:                 60.0 kHz
  // Rx bandwidth:                        250.0 kHz
  // output power:                        7 dBm
  // preamble length:                     32 bits
  state = radio1.begin(434.0, 32.0, 60.0, 250.0, 7, 32);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }


 // you can also change the settings at runtime
  // and check if the configuration was changed successfully

  // set carrier frequency to 433.5 MHz
  if (radio1.setFrequency(433.5) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("[CC1101] Selected frequency is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set bit rate to 100.0 kbps
  state = radio1.setBitRate(100.0);
  if (state == RADIOLIB_ERR_INVALID_BIT_RATE) {
    Serial.println(F("[CC1101] Selected bit rate is invalid for this module!"));
    while (true) { delay(10); }
  } else if (state == RADIOLIB_ERR_INVALID_BIT_RATE_BW_RATIO) {
    Serial.println(F("[CC1101] Selected bit rate to bandwidth ratio is invalid!"));
    Serial.println(F("[CC1101] Increase receiver bandwidth to set this bit rate."));
    while (true) { delay(10); }
  }

  // set receiver bandwidth to 250.0 kHz
  if (radio1.setRxBandwidth(250.0) == RADIOLIB_ERR_INVALID_RX_BANDWIDTH) {
    Serial.println(F("[CC1101] Selected receiver bandwidth is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set allowed frequency deviation to 10.0 kHz
  if (radio1.setFrequencyDeviation(10.0) == RADIOLIB_ERR_INVALID_FREQUENCY_DEVIATION) {
    Serial.println(F("[CC1101] Selected frequency deviation is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set output power to 5 dBm
  if (radio1.setOutputPower(5) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println(F("[CC1101] Selected output power is invalid for this module!"));
    while (true) { delay(10); }
  }

  // 2 bytes can be set as sync word
  if (radio1.setSyncWord(0x01, 0x23) == RADIOLIB_ERR_INVALID_SYNC_WORD) {
    Serial.println(F("[CC1101] Selected sync word is invalid for this module!"));
    while (true) { delay(10); }
  }
*/

