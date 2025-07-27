#pragma once
#ifndef SDFS_H
#define SDFS_H

#include "Wiring.h"
#include <SD.h>
#include <SPI.h>
#include <cstdint>

// Constants
extern const char* NFC_BASE_DIR;
extern const char* NFC_EXT;

// Functions
bool initSDCard();
bool ensureNfcDirExists();
bool saveNfcData(const uint8_t* rawData, size_t dataSize);

// File type enum
enum class FileType {
    Unknown,
    Document,
    Executable,
    Interpretable,
    Audio,
    Video,
    Config,
    Image,
    RawData 
};

// Helper functions
bool iequals(const char* a, const char* b);
const char* getFileExtension(const char* filename);
FileType GetFileType(const char* extension);
FileType GetFileTypeFromName(const char* filename);

#endif // SDFS_H
