// NFCManager.cpp
#include "NFC.h"
#include <Arduino.h>

NFCManager::NFCManager(uint8_t irqPin, uint8_t resetPin) : currentMode(NFC_OFF), uidLength(0) {

    memset(lastUid, 0, sizeof(lastUid));

    nfc = new Adafruit_PN532(irqPin, resetPin);
}

NFCManager::~NFCManager() {
    delete nfc;
}

void NFCManager::begin() {
    nfc->begin();
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata) {
        Serial.println("PN532 not found");
        return;
    }
    nfc->SAMConfig();
    setMode(NFC_OFF);
}

void NFCManager::setMode(NFC_MODE newMode) {
  
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
            // nfc->powerDown();
            break;
        case NFC_PLAYBACK:
            // nfc->powerUp();
            break;
        case NFC_RECORD:
            // nfc->powerUp();
            break;
    }
    
    currentMode = newMode;
}

void NFCManager::update() {
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

void NFCManager::handlePlayback() {
    if (nfc->inListPassiveTarget()) {
        uint8_t ndefMessage[] = { /* your NDEF data */ };
        // nfc->ntag2xx_WriteNDEF(ndefMessage, sizeof(ndefMessage));
        Serial.println("NDEF message sent");
    }
}

void NFCManager::handleRecording() {
    if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, lastUid, &uidLength)) {
        Serial.println("Tag detected, reading...");
        // Read NDEF data here
    }
}