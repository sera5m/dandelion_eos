// NFCManager.h
#pragma once

#include <Adafruit_PN532.h>

typedef enum {
    NFC_OFF,
    NFC_PLAYBACK,
    NFC_RECORD
} NFC_MODE;

class NFCManager {
public:
    NFCManager(uint8_t irqPin, uint8_t resetPin);
    ~NFCManager();
    void begin();
    void setMode(NFC_MODE newMode);
    void update();

private:
    void handlePlayback();
    void handleRecording();

    Adafruit_PN532* nfc;
    NFC_MODE currentMode;
    uint8_t lastUid[7];
    uint8_t uidLength;
};