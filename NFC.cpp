#include "nfc.h"
#include <Arduino.h>

NFCManager::NFCManager(uint8_t irq, uint8_t NFC_RST_PIN) {
    nfc = new Adafruit_PN532(irq, NFC_RST_PIN);
}

NFCManager::~NFCManager() {
    delete nfc;
}

bool NFCManager::begin() {
    if (!nfc->begin()) {
        Serial.println("PN532 not found");
        return false;
    }
    
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata) return false;
    
    nfc->SAMConfig();
    return true;
}

void NFCManager::setMode(Mode newMode) {
    if (newMode == currentMode) return;
    
    // Reset state when changing modes
    lastUid.clear();
    tagWritten = false;
    currentMode = newMode;
}

bool NFCManager::isTagPresent() {
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;
    return nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10);
}

void NFCManager::update() {
    const uint32_t now = millis();
    if (now - lastCheck < 100) return; // 10 Hz update
    lastCheck = now;

    switch(currentMode) {
        case Mode::Read:
            handleReadMode();
            break;
        case Mode::Write:
            handleWriteMode();
            break;
        default: 
            // Off mode - no action
            break;
    }
}

void NFCManager::handleReadMode() {
    uint8_t uid[7];
    uint8_t uidLength;
    if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10)) {
        // Convert UID to vector
        lastUid.assign(uid, uid + uidLength);
        Serial.println("Tag detected");
    }
}

void NFCManager::handleWriteMode() {
    uint8_t uid[7];
    uint8_t uidLength;
    
    if (!nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 10)) {
        tagWritten = false;  // Reset flag when tag is removed
        return;
    }
    
    // Check if this is a new tag
    if (lastUid.size() != uidLength || 
        memcmp(lastUid.data(), uid, uidLength) != 0) {
        lastUid.assign(uid, uid + uidLength);
        tagWritten = false;
    }
    
    // Write to tag if not already written
    if (!tagWritten) {
        if (tryWriteTag(/* your data here */)) {
            tagWritten = true;
            Serial.println("Write successful");
        }
    }
}

bool NFCManager::tryWriteTag(const std::vector<uint8_t>& data) {
    // Example NTAG213 writing - adjust for your tag type
    uint8_t success;
    uint8_t pageBuffer[4] = {0};
    
    // Write data to pages (NTAG213 has 36 pages)
    for (size_t i = 0; i < data.size(); i += 4) {
        memcpy(pageBuffer, data.data() + i, min((size_t)4, data.size() - i));
        success = nfc->ntag2xx_WritePage(4 + i/4, pageBuffer); // Start from page 4
        
        if (!success) {
            Serial.print("Failed to write page ");
            Serial.println(4 + i/4);
            return false;
        }
    }
    return true;
}