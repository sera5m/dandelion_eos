#pragma once
#include <Adafruit_PN532.h>
#include <vector>

class NFCManager {
public:
    enum class Mode { Off, Read, Write,emulate };
    
    NFCManager(uint8_t irqPin, uint8_t resetPin);
    ~NFCManager();
    
    bool begin();
    void setMode(Mode newMode);
    bool tryReadTag(std::vector<uint8_t>& uid, std::vector<uint8_t>& data);
    bool tryWriteTag(const std::vector<uint8_t>& data);
    bool isTagPresent();
    void update();
    
private:
    void handleReadMode();
    void handleWriteMode();
    
    Adafruit_PN532* nfc;
    Mode currentMode = Mode::Off;
    uint32_t lastCheck = 0;
    std::vector<uint8_t> lastUid;
    bool tagWritten = false;
};