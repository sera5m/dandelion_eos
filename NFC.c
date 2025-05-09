//i'm getting lazy with ino include files we're doing a c file fuck that


//#include <Adafruit_PN532.h>
#include "Wiring.h"



//nfc: tiny low data 13.56 mhz communication,bidirectional, the shit in access passes
//rfid: cards that have things, one way, active storage



//subsection=================handle hardware

// Create an instance of the PN532 using hardware SPI
//Adafruit_PN532 nfc(SPI, SPI_NFC_CS);
/*
// Setup NFC
void setup_nfc() {
  Serial.begin(115200);
  Serial.println("Initializing PN532...");

  nfc.begin();
  
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN532 module");
    while (1);
  }

  // Set up NFC to read cards (set up SAM configuration)
  nfc.SAMConfig();
  Serial.println("PN532 Initialized.");
}





//read data from the card

//tag data retrieval
void read_string_from_tag(uint8_t startBlock, uint8_t numBlocks) {
  char result[4 * numBlocks + 1]; // +1 for null terminator
  int idx = 0;

  for (uint8_t i = 0; i < numBlocks; i++) {
    uint8_t buffer[4];
    if (!read_ntag_block(startBlock + i, buffer)) {
      Serial.println("Read failed.");
      return;
    }
    for (int j = 0; j < 4; j++) {
      result[idx++] = (char)buffer[j];
    }
  }
  result[idx] = '\0';
  Serial.print("Read string: ");
  Serial.println(result);
}


// Try reading a card
void try_read_card(bool SaveToSDCARD) {
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  // Check if a card is present
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    Serial.println("Found a card!");

    // Output UID data
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) {
      Serial.print(" 0x"); Serial.print(uid[i], HEX);
      //gotta cache data here by building the string
    }
    Serial.println("");
    //done with the for loop
    if (SaveToSDCARD){
      //save it with a name. prompt the user for a name and pull up the keyboard
    }

  } else {
    Serial.println("No card detected.");
  }


}



NTAG213 = 144 bytes usable (~36 blocks)

NTAG215 = 504 bytes (~126 blocks)

NTAG216 = 888 bytes (~222 blocks)

Don’t write to blocks 0–3. Start at block 4.


//write data to card
void write_string_to_tag(const char* str) {
  size_t len = strlen(str);
  uint8_t block = 4; // user memory starts at block 4

  while (len > 0) {
    uint8_t data[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < 4 && *str; i++) {
      data[i] = *str++;
      len--;
    }
    if (!write_ntag_block(block++, data)) {
      Serial.println("Write failed.");
      return;
    }
  }
  Serial.println("String written to tag.");
}
*/


//store data-here to the microsd or ram or whatever
