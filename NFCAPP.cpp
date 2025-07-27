#include "nfcapp.h"    // includes types.h, Wiring.h, etc.
#include <SD.h>        // if you need SD here
#include "InputHandler.h"
// Remove these lines:
// #include "Micro2d_A.ino"
// #include "SDFS.ino"
// #include "ESPwatchv4.ino"
//never ever include.ino

NFCApp::NFCApp() : nfc(std::make_shared<NFCManager>(IRQ_PIN, RST_PIN)) {}

void NFCApp::init() {
    nfc->begin();
    transition(NFCAppMode::Off);
}

void NFCApp::transition(NFCAppMode newMode) {
    // Cleanup previous mode
    switch(currentMode) {
        case NFCAppMode::Reading:
            nfc->setMode(NFCManager::Mode::Off);
            break;
        case NFCAppMode::Writing:
            nfc->setMode(NFCManager::Mode::Off);
            break;
    }

    currentMode = newMode;
    navPosition = 0;

    // Initialize new mode
    switch(newMode) {
        case NFCAppMode::Reading:
            nfc->setMode(NFCManager::Mode::Read);
            break;
        case NFCAppMode::Writing:
            nfc->setMode(NFCManager::Mode::Emulate);
            break;
        case NFCAppMode::Loading:
            loadTagList();
            updateNavLimits();
            break;
    }
}

void NFCApp::update() {
    if (!nfc) return;

    switch(currentMode) {
        case NFCAppMode::Reading:
            if (nfc->tryReadTag(currentUid, currentData)) {
                // Process read data
            }
            break;
            
        case NFCAppMode::Writing:
            // Handle emulation updates
            break;
    }
}
std::string NFCApp::generateMenuContent() {
    std::string content = "Select Mode:\n";
    const char* modes[] = {"1. Read Tag", "2. Write Tag", "3. Load Tags", "4. Settings"};
    
    for(uint8_t i = 0; i <= navState.maxPosition; i++) {
        content += (i == navState.position) ? "> " : "  ";
        content += modes[i];
        content += "\n";
    }
    return content;
}

void NFCApp::updateUI() {
    std::string content;
    
    if(!navState.inSubMenu) {
        content = generateMenuContent();
    } else {
        switch(currentMode) {
            case NFCAppMode::Reading:
                content = "Scanning...\n";
                if(!lastUid.empty()) {
                    content += "UID: ";
                    for(auto b : lastUid) {
                        content += String(b, HEX).c_str();
                        content += " ";
                    }
                }
                break;
                
            case NFCAppMode::Writing:
                content = tagWritten ? "Write successful!\n" : "Place tag to write\n";
                break;
                
            // Other modes...
        }
    }
    
    Win_GeneralPurpose->updateContent(content);
}

void NFCApp::handleInput(uint16_t key) {
    switch(key) {
        case key_up:
            navState.position = (navState.position > 0) ? 
                navState.position - 1 : navState.maxPosition;
            break;
            
        case key_down:
            navState.position = (navState.position < navState.maxPosition) ? 
                navState.position + 1 : 0;
            break;
            
        case key_enter:
            if(!navState.inSubMenu) {
                transition(static_cast<NFCAppMode>(navState.position + 1));
                navState.inSubMenu = true;
                navState.maxPosition = /* Set based on mode */;
            } else {
                // Handle mode-specific actions
            }
            break;
            
        case key_back:
            if(navState.inSubMenu) {
                transition(NFCAppMode::Off);
                navState.inSubMenu = false;
                navState.maxPosition = 3; // Reset to main menu
            }
            break;
    }
    updateUI();
}
void NFCApp::setModeNavigation(NFCAppMode mode) {
    switch(mode) {
        case NFCAppMode::Loading:
            navState.maxPosition = min(tagList.size(), NFC_TAG_NAMES_MAX_DISPLAYABLE) - 1;
            break;
        case NFCAppMode::Settings:
            navState.maxPosition = 4; // Number of settings options
            break;
        default:
            navState.maxPosition = 0; // Most modes don't need vertical nav
    }
}

void NFCApp::render() {
    std::string content;
    
    switch(currentMode) {
        case NFCAppMode::Off:
            content = "NFC Tools\n"
                      "1. Read Tag\n"
                      "2. Write Tag\n"
                      "3. Load Tags";
            break;
            
        case NFCAppMode::Reading:
            content = "Scanning...\n";
            if (!currentUid.empty()) {
                content += "Tag detected!";
            }
            break;
            
        // ... other modes ...
    }
    
    Win_GeneralPurpose->updateContent(content);
}

void nfcTask(void* pvParameters) {
    NFCManager* nfc = static_cast<NFCManager*>(pvParameters);
    while(true) {
        nfc->update();
        
        // Optional: Notify main thread of changes
        if(nfc->stateChanged()) {
            xTaskNotify(uiTaskHandle, NFC_UPDATE, eSetBits);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}