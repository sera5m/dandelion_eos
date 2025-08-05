#include "SDFS.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <Arduino.h>  // for Serial etc
#include "InputHandler.h"
// Define constants
const char* NFC_BASE_DIR = "/main/rec/nfc";
const char* NFC_EXT = ".nfcd";

// Initialize the SD card
bool initSDCard() {
    SPIClass spiBus(HSPI); // Use HSPI (or VSPI) for SD card

    for (int attempt = 0; attempt < 3; attempt++) {
        if (SD.begin(SPI_CS_SD, spiBus)) {
            return true;
        }
        delay(500);
    }

    return false;
}

// Ensure NFC directory exists
bool ensureNfcDirExists() {
    if (!SD.exists(NFC_BASE_DIR)) {
        return SD.mkdir(NFC_BASE_DIR);
    }
    return true;
}

// Save raw NFC data to a file
bool saveNfcData(const uint8_t* rawData, size_t dataSize) {
    if (!rawData || dataSize == 0) return false;

    if (!ensureNfcDirExists()) {
        Serial.println("Failed to create NFC dir!");
        return false;
    }

    char filename[64];
    for (int i = 1; i < 1000; i++) {
        snprintf(filename, sizeof(filename), "%s/nfc_%03d%s", NFC_BASE_DIR, i, NFC_EXT);
        if (!SD.exists(filename)) {
            break;
        }
    }

    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to create file!");
        return false;
    }

    size_t bytesWritten = file.write(rawData, dataSize);
    file.close();

    if (bytesWritten != dataSize) {
        Serial.println("Write incomplete!");
        return false;
    }

    Serial.printf("Saved NFC data to: %s (%zu bytes)\n", filename, bytesWritten);
    return true;
}

// Case-insensitive string compare
bool iequals(const char* a, const char* b) {
    return strcasecmp(a, b) == 0;
}

// Extract file extension
const char* getFileExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

// Map file extension to FileType
FileType GetFileType(const char* extension) {
    if (!extension || *extension == '\0') return FileType::Unknown;

    static const struct {
        const char* ext;
        FileType type;
    } extensionMap[] = {
        {"csv", FileType::Document},
        {"txt", FileType::Document},
        {"mp3", FileType::Audio},
        {"ogg", FileType::Audio},
        {"wav", FileType::Audio},
        {"mp4", FileType::Video},
        {"gif", FileType::Video},
        {"avi", FileType::Video},
        {"png", FileType::Image},
        {"jpg", FileType::Image},
        {"bmp", FileType::Image},
        {"ico", FileType::Image},
        {"TDPF", FileType::Audio},
        {"cfg", FileType::Config},
        {"nfcd", FileType::RawData},
        {nullptr, FileType::Unknown}
    };

    for (const auto* entry = extensionMap; entry->ext; ++entry) {
        if (iequals(extension, entry->ext)) {
            return entry->type;
        }
    }

    return FileType::Unknown;
}

// Get file type by filename
FileType GetFileTypeFromName(const char* filename) {
    return GetFileType(getFileExtension(filename));
}




