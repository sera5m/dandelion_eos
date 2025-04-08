#ifndef NFCNRFID_INO
#define NFCNRFID_INO
/*
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include "PN532_debug.h" //debug lib?
#include "snep.h" //what is this
#include "NdefMessage.h"

extern int SPI_CLK_SPEED; //the devices default spi multiplier

PN532_SPI pn532spi(SPI, 10); //don't have a goddamn clue but this is called on setup
NfcAdapter nfc = NfcAdapter(pn532spi);
SNEP nfc(pn532spi); //idk what this is but its needed
uint8_t ndefBuf[128];

//use this library because it explicitly says it fixed the hello and then stuck bug https://github.com/elechouse/PN532?tab=readme-ov-file

//notes to future


//wipe a tag

    Serial.println("\nPlace a tag on the NFC reader to clean.");

    if (nfc.tagPresent()) {

        bool success = nfc.clean();
        if (success) {
            Serial.println("\nSuccess, tag restored to factory state.");
        } else {
            Serial.println("\nError, unable to clean tag.");
        }

//erase
nfc.erase()
//format
 nfc.format();
nfc.read();
 tag.print();

https://github.com/don/NDEF/blob/master/examples/ReadTagExtended/ReadTagExtended.ino

//this block of code to write to the tag with multiple records
        NdefMessage message = NdefMessage();
        message.addTextRecord("Hello, Arduino!");
        message.addUriRecord("http://arduino.cc");
        message.addTextRecord("Goodbye, Arduino!");
        boolean success = nfc.write(message);


https://github.com/don/NDEF/blob/master/examples/WriteTag/WriteTag.ino

//ok p2p recieve example 
void loop() {
    Serial.println("Waiting for message from Peer");
    int msgSize = nfc.read(ndefBuf, sizeof(ndefBuf));
    if (msgSize > 0) {
        NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
        msg.print();
        Serial.println("\nSuccess");
    } else {
        Serial.println("Failed");
    }
    delay(3000);
}



//recieve

void loop() {
    Serial.println("Send a message to Peer");
    
    NdefMessage message = NdefMessage();
    message.addUriRecord("http://shop.oreilly.com/product/mobile/0636920021193.do");
    //message.addUriRecord("http://arduino.cc");
    //message.addUriRecord("https://github.com/don/NDEF");

    
    int messageSize = message.getEncodedSize();
    if (messageSize > sizeof(ndefBuf)) {
        Serial.println("ndefBuf is too small");
        while (1) {
        }
    }

    message.encode(ndefBuf);
    if (0 >= nfc.write(ndefBuf, messageSize)) {
        Serial.println("Failed");
    } else {
        Serial.println("Success");
    }

    delay(3000);
}










*/




#endif