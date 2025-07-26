//
#include "types.h"
#include "NFC.h"
#include <iostream>
// app nfc===================================================================================================================================================================================// NFC globals
std::shared_ptr<NFCManager> nfc_manager; // Keeps alive
TaskHandle_t nfcTaskHandle = NULL;
static std::shared_ptr<Window> nfcStatusWindow;
//Win_GeneralPurpose used here for lazy reasons

void setupNFCUI() {
  nfcStatusWindow = std::make_shared<Window>("nfc_app", nfcStatusCfg, "NFC: OFF");
  windowManagerInstance->registerWindow(nfcStatusWindow);
 nfc_manager = std::make_shared<NFCManager>(IRQ, NFC_RST_PIN);

}

void start_nfc_task() {
  if (nfc_manager == nullptr) {
    Serial.println("[ERROR] NFC Manager is null!");
    return;
  }

  xTaskCreatePinnedToCore(
    nfcTask,
    "NFCTask",
    8192,
    nfc_manager.get(),  // Pass raw pointer
    1,
    &nfcTaskHandle,
    1
  );
}


//logic variables
nfcAppMode NFC_APP_currentmode;
char myStr[16]; //note last char is \o
//ame based on the tag itself idk

//config variables, not even user ones, may be mutated by logic, fuck you, die
//put in a struct bc nice n clean :3
struct nfc_app_cfg{
bool savetoSDcard=true;//if not we save to uhhh internal which is stupid and bad and bad and stupid and bad and stupid and bad translation: (required, but i personally don't like it)
uint16_t maxCardsStorable=100;//why will you need more than this? -used in microsd card alloc
bool 
}

//app activated with void transitionapp
void NFC_APP_INIT(){
rst_nav_pos();
Win_GeneralPurpose->updateContent("");
Win_GeneralPurpose->ResizeWindow(128, 128,false);  Win_GeneralPurpose->MoveWindow(0,0,0);

}//end task void watchscreen

void NFC_APP_EXIT(){
    rst_nav_pos();
Win_GeneralPurpose->updateContent("");

}//switch back to watch screen

//swap between modes of this app
void NFC_APP_TRANSITION(nfcAppMode newmode){ //no, not starting mode aware, no need. die
rst_nav_pos();
Win_GeneralPurpose->updateContent(""); //clear

switch (newmode){
case nam_reading :
break;
case nam_writing:
break;
case nam_sleep:
break;
case nam_saving:
break;
nam_loadfromStorage:
break;    

default:
break;
}
tft.fillScreen(tcol_background);
}

//fuck me, what  was the mouse logic

//WatchScreenUpdateInterval=350; idk what update interval
// Navlimits_ will need to be set here depending on mode!
//use navpos for all inputs to go with new system

void input_handler_fn_NFCAPP(uint16_t key){
    switch (key){


    case key_left:
    break;
    case key_right:
    break;
    case key_up:
    break;
    case key_down:
    break;
    case key_enter:
    break:
    case key_back:
    break;    
    
    default:
    break;
    }

}

void on_nfc_logic_tick(nfcAppMode NFC_APP_currentmode){

switch (NFC_APP_currentmode){
case nam_reading: //scan tick and READ THE CARD
break;
case nam_writing: //write data to the card
break;
case nam_sleep: //nothing is happening, leave it be, off by default
break;
case nam_saving: //saving to the storage
//tell user to wait to save it 
break;
nam_loadfromStorage: //load from storage
break;    

default:
break;
}
}
//maybe a static void for running idk
//create the text for the screen itself
std::string create_nfc_app_text(){

}

void NFC_APP_renderLoop(){

}




void end_nfc_task() {
  if (nfcTaskHandle) {
    vTaskDelete(nfcTaskHandle);
    nfcTaskHandle = NULL;
  }
}



//warning: below this is old bad code!

void nfcTask(void *pvParameters) {
  NFCManager* local_manager = static_cast<NFCManager*>(pvParameters);
  uint8_t uid[7];
  uint8_t uidLength;
  char NFC_filepath[80];

  while (1) {
    Serial.println("[NFC] Scanning...");

    bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200);

    if (!success) {
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    Serial.print("[NFC] Found Tag UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.printf("%02X ", uid[i]);
    }
    Serial.println();

    snprintf(NFC_filepath, sizeof(NFC_filepath), "/nfc");
    if (!SD.exists(NFC_filepath)) {
      SD.mkdir(NFC_filepath);
    }

    snprintf(NFC_filepath, sizeof(NFC_filepath),
             "/nfc/%02X%02X%02X%02X%02X%02X%02X.nfcdat",
             uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);

    File dataFile = SD.open(NFC_filepath, FILE_WRITE);
    if (!dataFile) {
      Serial.println("[SD] Couldn't open file to write");
    } else {
      Serial.println("[SD] Writing tag data...");

      uint8_t data[4];
      for (uint8_t page = 0; page < 42; page++) {
        success = nfc.ntag2xx_ReadPage(page, data);
        if (success) {
          dataFile.printf("PAGE %02d: %02X %02X %02X %02X\n",
                          page, data[0], data[1], data[2], data[3]);
        } else {
          dataFile.printf("PAGE %02d: Read Error\n", page);
        }
      }
      dataFile.close();
      Serial.printf("[SD] Saved: %s\n", NFC_filepath);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

