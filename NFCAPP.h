#pragma once
#include <SD.h>        // OK if needed
#include "nfc.h"
#include "types.h"
#include "Wiring.h"
// Remove: #include "sdfs.ino"


enum class NFCAppMode {
    Off,
    Reading,
    Writing,
    Saving,
    Loading
};
// In NFCApp.h
struct NavState {
    uint8_t position = 0;
    uint8_t maxPosition = 3; // For main menu
    bool inSubMenu = false;
};

class NFCApp {
public:
    NFCApp();
    void init();
    void exit();
    void transition(NFCAppMode newMode);
    void handleInput(uint16_t key);
    void update();
    void render();

private:
    void updateNavLimits();
    void saveCurrentTag();
    void loadTagList();
    
    std::shared_ptr<NFCManager> nfc;
    NFCAppMode currentMode = NFCAppMode::Off;
    
    // Navigation state
    uint8_t navPosition = 0;
    uint8_t navMaxPosition = 0;
    
    // Tag data
    std::vector<uint8_t> currentUid;
    std::vector<uint8_t> currentData;
    char currentTagName[18] = {0};
    
    // Configuration
    struct Config {
        bool saveToSD = true;
        uint16_t maxCards = 100;
    } config;
};