// nfcapp.cpp
#include "nfcapp.h"
#include "types.h"
// --- PN532 pins from Wiring.h ---

extern Adafruit_PN532 nfc;

// Global app state
NFCAppState nfcAppState;



static void nfc_setMode(NFCMode m) {
  if (m == nfcAppState.nfcMode) return;
  nfcAppState.lastUid.clear();
  nfcAppState.tagWritten = false;
  nfcAppState.nfcMode = m;
}

static bool nfc_isTagPresent() {
  uint8_t uidBuf[7], len;
  return nfcAppState.nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuf, &len, 10);
}

static bool nfc_tryReadTag(std::vector<uint8_t>& uid, std::vector<uint8_t>& data) {
  // deliver lastUid+data once
  if (nfcAppState.lastUid.empty() || data.empty()) return false;
  uid = nfcAppState.lastUid;
  return true;
}

static bool nfc_tryWriteTag(const std::vector<uint8_t>& data) {
  uint8_t pageBuf[4];
  for (size_t i = 0; i < data.size(); i += 4) {
    size_t chunk = min((size_t)4, data.size() - i);
    memset(pageBuf, 0, 4);
    memcpy(pageBuf, data.data()+i, chunk);
    uint8_t page = 4 + i/4;
    if (!nfcAppState.nfc->ntag2xx_WritePage(page, pageBuf)) {
      Serial.print("Fail @page "); Serial.println(page);
      return false;
    }
  }
  return true;
}

static void nfc_update() {
  uint32_t now = millis();
  if (now - nfcAppState.lastCheck < 100) return;
  nfcAppState.lastCheck = now;

  uint8_t uidBuf[7], len;
  bool present = nfcAppState.nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuf, &len, 10);

  switch (nfcAppState.nfcMode) {
    case NFC_MODE_READ:
      if (present) {
        nfcAppState.lastUid.assign(uidBuf, uidBuf + len);
        Serial.println("Tag detected");
      }
      break;

    case NFC_MODE_WRITE:
      if (!present) {
        nfcAppState.tagWritten = false;
        break;
      }
      if (nfcAppState.lastUid.size()!=len ||
          memcmp(nfcAppState.lastUid.data(), uidBuf,len)!=0) {
        nfcAppState.lastUid.assign(uidBuf, uidBuf + len);
        nfcAppState.tagWritten = false;
      }
      if (!nfcAppState.tagWritten &&
          nfc_tryWriteTag(nfcAppState.pendingWriteData)) {
        nfcAppState.tagWritten = true;
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
  delete nfcAppState.nfc;
  nfcAppState = NFCAppState();
}

void NFC_APP_TRANSITION(NFCAppMode newMode) {
  // turn off reader/writer
  if (nfcAppState.currentMode==NAM_READING ||
      nfcAppState.currentMode==NAM_WRITING) {
    nfc_setMode(NFC_MODE_OFF);
  }

  // mode-specific inits
  switch (newMode) {
    case NAM_OFF:
      nfcAppState.navPosition = 0;
      nfcAppState.navMaxPosition = 3;
      Navlimits_ = {nfcAppState.navMaxPosition,1,0};
      break;

    case NAM_READING:
      nfc_setMode(NFC_MODE_READ);
      nfcAppState.currentUid.clear();
      nfcAppState.currentData.clear();
      Navlimits_ = {0,0,0};
      break;

    case NAM_WRITING:
      nfc_setMode(NFC_MODE_WRITE);
      Navlimits_ = {0,0,0};
      break;

    case NAM_LOADING:
      // load file list…
      nfcAppState.tagFiles.clear();
      {
        File root = SD.open("/nfc");
        while (File f = root.openNextFile()) {
          if (!f.isDirectory()) nfcAppState.tagFiles.push_back(f.name());
          f.close();
        }
        root.close();
      }
      nfcAppState.navPosition = 0;
      nfcAppState.navMaxPosition = min<size_t>
         (nfcAppState.tagFiles.size(), nfcAppState.config.maxCards)-1;
      Navlimits_ = {nfcAppState.navMaxPosition,1,0};
      break;

    default: break;
  }

  nfcAppState.currentMode = newMode;
  globalNavPos = {0,0,0};
}

void input_handler_fn_NFCAPP(uint16_t key) {
  // unchanged…
}

void NFC_APP_UPDATE() {
  nfc_update();

  switch (nfcAppState.currentMode) {
    case NAM_READING:
      if (nfc_tryReadTag(nfcAppState.currentUid, nfcAppState.currentData)) {
        NFC_APP_TRANSITION(NAM_SAVING);
      }
      break;
    case NAM_WRITING:
      // pendingWriteData should be set elsewhere before entering
      break;
    case NAM_SAVING:
      // save to SD or internal…
      NFC_APP_TRANSITION(NAM_OFF);
      break;
    default: break;
  }
}

void NFC_APP_RENDER() {
  // uh oh i lost the original code todo fix this shit!!
}

void nfcTask(void* pvParameters) {
    Serial.println("NFC Task starting...");

    // Commented out to isolate crash cause
    // NFC_APP_TRANSITION(NAM_READING);

    while (true) {
        Serial.println("NFC task alive");
        // NFC_APP_UPDATE();
        vTaskDelay(pdMS_TO_TICKS(5000)); // print every 5 seconds
    }
}


