#ifndef NFCAPP_H
#define NFCAPP_H

// nfcapp.h
#pragma once

#include <Arduino.h>
#include <SD.h>
#include <Adafruit_PN532.h>
#include <vector>
#include <memory>
#include "types.h"
#include "Wiring.h"
#include "InputHandler.h"

extern Adafruit_PN532 nfc;

// --- NFC “module” modes (was in nfc.h) ---
typedef enum {
  NFC_MODE_OFF,
  NFC_MODE_READ,
  NFC_MODE_WRITE,
  NFC_MODE_EMULATE
} NFCMode;

// App modes
enum NFCAppMode {
  NAM_OFF,
  NAM_READING,
  NAM_WRITING,
  NAM_SAVING,
  NAM_LOADING
};

// Global state
struct NFCAppState {
  // app
  NFCAppMode currentMode = NAM_OFF;
  // NFC driver state
  NFCMode    nfcMode     = NFC_MODE_OFF;
  uint32_t   lastCheck   = 0;
  std::vector<uint8_t> lastUid;
  bool       tagWritten  = false;
  std::vector<uint8_t> pendingWriteData;
  Adafruit_PN532* nfc;          // pointer to PN532 instance

  // navigation
  uint8_t navPosition = 0;
  uint8_t navMaxPosition = 3;

  // tag data
  std::vector<uint8_t> currentUid;
  std::vector<uint8_t> currentData;
  char currentTagName[18] = {0};

  // SD + config
  struct { bool saveToSD = true; uint16_t maxCards = 100; } config;
  std::vector<String> tagFiles;
  uint8_t fileListOffset = 0;
};

extern NFCAppState nfcAppState;

// App interface (unchanged signatures)
void NFC_APP_INIT();
void NFC_APP_EXIT();
void NFC_APP_TRANSITION(NFCAppMode newMode);
void input_handler_fn_NFCAPP(uint16_t key);
void NFC_APP_UPDATE();
void NFC_APP_RENDER();
extern void nfcTask(void* pvParameters);
#endif