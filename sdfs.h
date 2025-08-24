#pragma once
#ifndef SDFS_H
#define SDFS_H

#include "SDFS.h"
#include <Arduino.h>
#include "Wiring.h"
#include <SD.h>
#include <SPI.h>
#include <cstdint>
#include <FS.h>          // for fs::FS, File
#include "InputHandler.h"

// =============================
// Constants
// =============================
extern const char* NFC_BASE_DIR;
extern const char* NFC_EXT;


#define FS_MAX_ITEMS_PAGE 10 //128 px, giving 12.8-13 px per item



//enums==================

enum FileType {
    FILETYPE_UNKNOWN,
    FILETYPE_DOCUMENT,
    FILETYPE_AUDIO,
    FILETYPE_VIDEO,
    FILETYPE_IMAGE,
    FILETYPE_CONFIG,
    FILETYPE_RAWDATA
};

//===================functions==================================//










// SD card init
bool initSDCard();

// Directory management
bool ensureNfcDirExists();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void listDirPaged(fs::FS &fs, const char *dirname, uint16_t startIndex, uint16_t count, 
                  char *output, size_t outputSize, uint8_t fileTypeFilter = 0);
void createDir(fs::FS &fs, const char *path);
void removeDir(fs::FS &fs, const char *path);

// File operations
void readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void renameFile(fs::FS &fs, const char *path1, const char *path2);
void deleteFile(fs::FS &fs, const char *path);
void testFileIO(fs::FS &fs, const char *path);

// NFC-specific
bool saveNfcData(const uint8_t* rawData, size_t dataSize);

// Utilities
inline bool iequals(const char* a, const char* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    
    while (*a && *b) {
        if (tolower(*a) != tolower(*b))
            return false;
        a++;
        b++;
    }
    return *a == *b;
}

// Get file extension without dot
inline const char* getFileExtension(const char* filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void listDirPaged(fs::FS &fs, const char *dirname, uint16_t startIndex, uint16_t count, 
                  char *output, size_t outputSize, uint8_t fileTypeFilter = 0,
                  const char *extensionFilter = nullptr);

FileType GetFileType(const char* extension);
FileType GetFileTypeFromName(const char* filename);


bool extensionMatches(const char *filename, const char *extension);
void listDirPaged(fs::FS &fs, const char *dirname, uint16_t startIndex, uint16_t count,char *output, size_t outputSize, uint8_t fileTypeFilter = 0,
                  const char *extensionFilter = nullptr);  // Add default here



#endif // SDUTILS_H
