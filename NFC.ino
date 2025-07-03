//i'm getting lazy with ino include files. laugh at me i know.  

//#include <Adafruit_PN532.h>
#include "Wiring.h"



//nfc: tiny low data 13.56 mhz communication,bidirectional, the shit in access passes
//rfid: cards that have things, one way, active storage

#include <Adafruit_PN532.h>


class NFCManager {
private:
    Adafruit_PN532* nfc;
    NFC_MODE currentMode = NFC_OFF;
    uint8_t lastUid[7] = {0};
    uint8_t uidLength = 0;
    
public:
    NFCManager(uint8_t irqPin, uint8_t resetPin) {
        nfc = new Adafruit_PN532(irqPin, resetPin);
    }

    void begin() {
        nfc->begin();
        uint32_t versiondata = nfc->getFirmwareVersion();
        if (!versiondata) {
            Serial.println("PN532 not found");
            return;
        }
        nfc->SAMConfig();
        setMode(NFC_OFF);
    }

    void setMode(NFC_MODE newMode) {
        if (newMode == currentMode) return;
        
        // Cleanup previous mode
        switch(currentMode) {
            case NFC_RECORD:
                // Any recording cleanup
                break;
            case NFC_PLAYBACK:
                // Any playback cleanup
                break;
        }
        
        // Initialize new mode
        switch(newMode) {
            case NFC_OFF:
              //  nfc->powerDown();
                break;
            case NFC_PLAYBACK:
              //  nfc->powerUp();
                break;
            case NFC_RECORD:
              //  nfc->powerUp();
                break;
        }
        
        currentMode = newMode;
    }

    void update() {
        static uint32_t lastCheck = 0;
        const uint32_t checkInterval = (currentMode == NFC_OFF) ? 1000 : 100;
        
        if (millis() - lastCheck < checkInterval) return;
        lastCheck = millis();

        switch(currentMode) {
            case NFC_PLAYBACK:
                handlePlayback();
                break;
            case NFC_RECORD:
                handleRecording();
                break;
            case NFC_OFF:
                // Minimal power usage
                break;
        }
    }

private:
    void handlePlayback() {
        // Example: When a phone taps, send pre-programmed NDEF message
        if (nfc->inListPassiveTarget()) {
            uint8_t ndefMessage[] = { /* your NDEF data */ };
       //     nfc->ntag2xx_WriteNDEF(ndefMessage, sizeof(ndefMessage));
            Serial.println("NDEF message sent");
        }
    }

    void handleRecording() {
        if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, lastUid, &uidLength)) {
            Serial.println("Tag detected, reading...");
            // Read NDEF data here
        }
    }
};

//subsection=================handle hardware

// Create an instance of the PN532 using hardware SPI
//Adafruit_PN532 nfc(SPI, SPI_NFC_CS); //PLACED IN MAIN

// Setup NFC


//read data from the card
/*
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


 write data to card
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
}*/



//store data-here to the microsd or ram or whatever
