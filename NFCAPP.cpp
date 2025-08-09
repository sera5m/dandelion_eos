// nfcapp.cpp
#include <Arduino.h>
#include "nfcapp.h"
#include "types.h"
#include "globals.h"
#include "s_hell.h"
#include "sdfs.h"
#include "Micro2D_A.h"
#include "InputHandler.h"
// --- PN532 pins from Wiring.h ---

extern Adafruit_PN532 nfc;

// Global app state
NFCAppState nfcAppState_current;
NFCAppMode nfcAppMODE_current=NAM_OFF; //
//nfc requires reference to external general purpose static alloc window
//this is for memory conservation due to the flexibility of my windows classes. we will simply attatch bitmap asets via canvas and assume controll

//-----------------grabbing window from globals.cpp and assuming controll for thsi app//
extern std::shared_ptr<Window> Win_GeneralPurpose; 

extern char watchscreen_buf[WATCHSCREEN_BUF_SIZE];

//--------------------------------------------------------------------/

//extern input handling---------------------
extern int16vect globalNavPos; //mouse position
extern int16vect Navlimits_; //how far the mouse is allowed to go
//------------------------------------------

//and this function rst_nav_pos();

//UI FLOW NOTES:
/*
USER PLACED in no mode by default, with menu, vertical scroll selection
pressing back on any mode's default screen sends them into the menu mode
while on menu mode, pressing enter swaps the app to that mode.  pressing back exits to main.

while in each mode, it follows typical save/load with back pushing you out of each



*/







const char* NFCModeNames[NFC_MODE_COUNT] = { 
  "Off    ",
  "Read   ",
  "Write  ",
  "Emulate"
};


static void nfc_setMode(NFCMode m) {
  if (m == nfcAppState_current.nfcMode) return;
  nfcAppState_current.lastUid.clear();
  nfcAppState_current.tagWritten = false;
  nfcAppState_current.nfcMode = m;
}

static bool nfc_isTagPresent() {
  uint8_t uidBuf[7], len;
  return nfcAppState_current.nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuf, &len, 10);
}

static bool nfc_tryReadTag(std::vector<uint8_t>& uid, std::vector<uint8_t>& data) {
  // deliver lastUid+data once
  if (nfcAppState_current.lastUid.empty() || data.empty()) return false;
  uid = nfcAppState_current.lastUid;
  return true;
}

static bool nfc_tryWriteTag(const std::vector<uint8_t>& data) {
  uint8_t pageBuf[4];
  for (size_t i = 0; i < data.size(); i += 4) {
    size_t chunk = min((size_t)4, data.size() - i);
    memset(pageBuf, 0, 4);
    memcpy(pageBuf, data.data()+i, chunk);
    uint8_t page = 4 + i/4;
    if (!nfcAppState_current.nfc->ntag2xx_WritePage(page, pageBuf)) {
      Serial.print("Fail @page "); Serial.println(page);
      return false;
    }
  }
  return true;
}

static void nfc_update() {
  uint32_t now = millis();
  if (now - nfcAppState_current.lastCheck < 100) return;
  nfcAppState_current.lastCheck = now;

  uint8_t uidBuf[7], len;
  bool present = nfcAppState_current.nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuf, &len, 10);

  switch (nfcAppState_current.nfcMode) {
    case NFC_MODE_READ:
      if (present) {
        nfcAppState_current.lastUid.assign(uidBuf, uidBuf + len);
        Serial.println("Tag detected");
      }
      break;

    case NFC_MODE_WRITE:
      if (!present) {
        nfcAppState_current.tagWritten = false;
        break;
      }
      if (nfcAppState_current.lastUid.size()!=len ||
          memcmp(nfcAppState_current.lastUid.data(), uidBuf,len)!=0) {
        nfcAppState_current.lastUid.assign(uidBuf, uidBuf + len);
        nfcAppState_current.tagWritten = false;
      }
      if (!nfcAppState_current.tagWritten &&
          nfc_tryWriteTag(nfcAppState_current.pendingWriteData)) {
        nfcAppState_current.tagWritten = true;
        Serial.println("Write OK");
      }
      break;

    default: break;
  }
}

// --- Application lifecycle ---

void NFC_APP_INIT() {
  nfc.begin();  

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    while (1); // hang
  }

  Serial.println("Found chip");
  nfc.SAMConfig();
}

void NFC_APP_EXIT() {
  nfc_setMode(NFC_MODE_OFF);
  delete nfcAppState_current.nfc;
  nfcAppState_current = NFCAppState();
}

void NFC_APP_TRANSITION(NFCAppMode newMode) {
  // turn off reader/writer
  if (nfcAppState_current.currentMode==NAM_READING ||
      nfcAppState_current.currentMode==NAM_WRITING) {
    nfc_setMode(NFC_MODE_OFF);
  }

  // mode-specific inits
  switch (newMode) {
    case NAM_OFF:
    Navlimits_ = {0, NFC_MODE_COUNT, 0};//user is in vertically navigated menu for this mode, and selects the mode from the list. ensure clamp to mode. 
      //nfcAppState_current.navPosition = 0;
      //nfcAppState_current.navMaxPosition = 3;
      
      break;

    case NAM_READING:
      nfc_setMode(NFC_MODE_READ);
      nfcAppState_current.currentUid.clear();
      nfcAppState_current.currentData.clear();
      Navlimits_ = {0,0,0};
      break;

    case NAM_WRITING:
      nfc_setMode(NFC_MODE_WRITE);
      Navlimits_ = {0,0,0};
      break;

    case NAM_LOADING:
      // load file list…
      nfcAppState_current.tagFiles.clear();
      {
        File root = SD.open("/nfc");
        while (File f = root.openNextFile()) {
          if (!f.isDirectory()) nfcAppState_current.tagFiles.push_back(f.name());
          f.close();
        }
        root.close();
      }
      nfcAppState_current.navPosition = 0;
      nfcAppState_current.navMaxPosition = min<size_t>
         (nfcAppState_current.tagFiles.size(), nfcAppState_current.config.maxCards)-1;
      Navlimits_ = {nfcAppState_current.navMaxPosition,1,0};
      break;

    default:
    Serial.print("invalid nfc app mode, mode is "); Serial.println(newMode); //i wish i could just 

     break;
  } 

  nfcAppState_current.currentMode = newMode;
  globalNavPos = {0,0,0};
}

void input_handler_fn_NFCAPP(uint16_t key) {
  // unchanged…
}

void NFC_APP_UPDATE() {
  //nfc_update();

  //switch (
}

//writes to watchscreenbuff using direct ref
//Navlimits_ = {0, NFC_MODE_COUNT, 0}; //limit mode selector to mode count while here-put in nfc app transition-SAFE

void nfcAppMakeLISTtext(uint8_t mousepos) {
    int offset = 0;
    watchscreen_buf[0] = '\0';

    for (int i = 0; i < NFC_MODE_COUNT; i++) {
        if (i == mousepos) {
            offset += snprintf(watchscreen_buf + offset,
                               WATCHSCREEN_BUF_SIZE - offset,
                               "<setcolor(0xFFFF)>%s<setcolor(0x00FF)>", NFCModeNames[i]);
        } else {
            offset += snprintf(watchscreen_buf + offset,
                               WATCHSCREEN_BUF_SIZE - offset,
                               "%s", NFCModeNames[i]);
        }

        if (i < NFC_MODE_COUNT - 1) {
            offset += snprintf(watchscreen_buf + offset,
                               WATCHSCREEN_BUF_SIZE - offset,
                               "<n>");
        }
    }
}



//follows chain implementation, mode passes here, render functions for each which require complex code
void NFC_APP_RENDER(NFCAppMode mode) {
  switch (mode){
  case NAM_OFF:
  
  //need event check for if allready updated this at least once to change background

  nfcAppMakeLISTtext(globalNavPos.y); //yes,yes,narrowing, we get it
  Win_GeneralPurpose->updateContent(watchscreen_buf);//

    break;

  case NAM_READING:
    //reading is a special case, pause screen updates if in safe mode to ensure no faulty data via bus conflicts
    //watchscreen_buf="/0"

    //Win_GeneralPurpose->updateContent("please wait!");
    Win_GeneralPurpose->updateContent("Reading...<n> Please wait!");
  break;

  case NAM_WRITING: //writing data to card, lock for 5s or untill works
    Win_GeneralPurpose->updateContent("writing data<n>pls wait");
  break;

  case NAM_SAVING: //is storing scanned card from reading mode to memory, block user interaction untill saved or 5s timeout
    Win_GeneralPurpose->updateContent("Saving card data...");
  break;

  case NAM_EMULATING: //device pretending to be nfc card
    Win_GeneralPurpose->updateContent("emulate mode, ");
  break;

  case NAM_LOADING: //please wait! loading the card, or user is in storage looking for their card to emulate/write. essentially file browser for them
    Win_GeneralPurpose->updateContent(" card list mode. pick one"); //todo: this will need to have a list, and loaded from the card list. shoudl i use an nindex table?
  
  break;

  default:
    Win_GeneralPurpose->updateContent("Unknown NFC mode");
    break;
  }
  WindowManager::getInstance().UpdateAllWindows(true,false);//i need to do better than this man
}



//need input handle for each



void NFC_APP_INPUT_menu(uint16_t key){
switch (key){


//enter/back
case key_back: 

break;  

case key_enter: 

break;

//directions
case key_up: break;

case key_down: break;

case key_right: break;

case key_left: break;

default:

  break;
}//switch statement end

}//end fn

void NFC_APP_INPUT_NAM_READING(uint16_t key){
  switch (key){
      case key_back: break;
      case key_enter: break;
      case key_up: break;
      case key_down: break;
      case key_right: break;
      case key_left: break;
      default: break;
  }
}

void NFC_APP_INPUT_NAM_WRITING(uint16_t key){
  switch (key){
      case key_back: break;
      case key_enter: break;
      case key_up: break;
      case key_down: break;
      case key_right: break;
      case key_left: break;
      default: break;
  }
}

void NFC_APP_INPUT_NAM_SAVING(uint16_t key){
  switch (key){

      case key_back: //exit to the list mode after saving it
      break;

      case key_enter: //fuck you want man we're saving it
       break;

      case key_up: break;
      case key_down: break;
      case key_right: break;
      case key_left: break;
      default: break;
  }
}

void NFC_APP_INPUT_NAM_LOADING(uint16_t key){
  switch (key){
      case key_back: break;
      case key_enter: break;
      case key_up: break;
      case key_down: break;
      case key_right: break;
      case key_left: break;
      default: break;
  }
}

void NFC_APP_INPUT_NAM_EMULATING(uint16_t key){
  switch (key){
      case key_back: break;
      case key_enter: break;
      case key_up: break;
      case key_down: break;
      case key_right: break;
      case key_left: break;
      default: break;
  }
}

void NFC_APP_INPUT_NAM_COUNT(uint16_t key){
  switch (key){
      case key_back: break;
      case key_enter: break;
      case key_up: break;
      case key_down: break;
      case key_right: break;
      case key_left: break;
      default: break;
  }
}






void nfcTask(void* pvParameters) {
    Serial.println("NFC Task starting...");
    NFC_APP_TRANSITION(NAM_OFF);//by default,off mode, ensure.
    // Commented out to isolate crash cause
    // NFC_APP_TRANSITION(NAM_READING);

    while (true) {
      NFC_APP_RENDER(nfcAppMODE_current); //take variable 

      
        
        vTaskDelay(pdMS_TO_TICKS(NFCAPP_TASKUPDATERATE_MS));  //go to the h file if you wanna
    }
}


