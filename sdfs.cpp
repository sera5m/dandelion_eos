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
    //probably wrong idk
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


void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

bool extensionMatches(const char *filename, const char *extension) {
  // Find last dot in filename
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename) return false; // No extension or hidden file
  
  // Compare extensions ignoring case
  const char *ext = dot + 1;
  while (*ext && *extension) {
      if (tolower(*ext) != tolower(*extension))
          return false;
      ext++;
      extension++;
  }
  
  // Both must reach end at same time
  return (*ext == '\0' && *extension == '\0');
}

void listDirPaged(fs::FS &fs, const char *dirname, uint16_t startIndex, uint16_t count, 
                  char *output, size_t outputSize, uint8_t fileTypeFilter,
                  const char *extensionFilter) {
    output[0] = '\0';  // Initialize output buffer
    
    File root = fs.open(dirname);
    if (!root) {
        snprintf(output, outputSize, "Failed to open directory");
        return;
    }
    
    if (!root.isDirectory()) {
        snprintf(output, outputSize, "Not a directory");
        root.close();
        return;
    }

    File file;
    uint16_t currentIndex = 0;
    uint16_t listedCount = 0;
    size_t currentLen = 0;

    while ((file = root.openNextFile()) && (count == 0 || listedCount < count)) {
        const char *name = file.name();
        bool isDirectory = file.isDirectory();
        
        // Apply file type filter
        bool include = true;
        if (fileTypeFilter == 1 && isDirectory) include = false;  // Skip directories
        if (fileTypeFilter == 2 && !isDirectory) include = false; // Skip files
        
        // Apply exact extension filter
        if (include && extensionFilter && !isDirectory) {
            const char *ext = getFileExtension(name);
            include = iequals(ext, extensionFilter);
        }

        if (include) {
            if (currentIndex >= startIndex) {
                size_t nameLen = strlen(name);
                size_t requiredLen = currentLen + nameLen + 2;  // +1 newline, +1 null

                if (requiredLen >= outputSize) {
                    file.close();
                    break;  // Buffer full
                }
                
                if (currentLen > 0) {
                    strcat(output, "\n");
                    currentLen++;
                }
                
                strcat(output, name);
                currentLen += nameLen;
                listedCount++;
            }
            currentIndex++;
        }
        file.close();
    }
    
    root.close();
}

void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char *path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %lu ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
  file.close();
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
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename) return "";
  return dot + 1;
}


FileType GetFileType(const char* extension) {
  if (!extension || *extension == '\0') return FILETYPE_UNKNOWN;

  if (iequals(extension, "csv") || iequals(extension, "txt")) {
      return FILETYPE_DOCUMENT;
  } else if (iequals(extension, "mp3") || iequals(extension, "ogg") || 
             iequals(extension, "wav") || iequals(extension, "TDPF")) {
      return FILETYPE_AUDIO;
  } else if (iequals(extension, "mp4") || iequals(extension, "gif") || 
             iequals(extension, "avi")) {
      return FILETYPE_VIDEO;
  } else if (iequals(extension, "png") || iequals(extension, "jpg") || 
             iequals(extension, "jpeg") || iequals(extension, "bmp") || 
             iequals(extension, "ico")) {
      return FILETYPE_IMAGE;
  } else if (iequals(extension, "cfg")) {
      return FILETYPE_CONFIG;
  } else if (iequals(extension, "nfcd")) {
      return FILETYPE_RAWDATA;
  }

  return FILETYPE_UNKNOWN;
}

FileType GetFileTypeFromName(const char* filename) {
  return GetFileType(getFileExtension(filename));
}

void listDirPaged(fs::FS &fs, const char *dirname, uint16_t startIndex, uint16_t count, 
                char *output, size_t outputSize, uint8_t fileTypeFilter, 
                FileType fileType) {
  output[0] = '\0';  // Initialize output buffer
  
  File root = fs.open(dirname);
  if (!root) {
      snprintf(output, outputSize, "Failed to open directory");
      return;
  }
  
  if (!root.isDirectory()) {
      snprintf(output, outputSize, "Not a directory");
      root.close();
      return;
  }

  File file;
  uint16_t currentIndex = 0;
  uint16_t listedCount = 0;
  size_t currentLen = 0;

  while ((file = root.openNextFile()) && (count == 0 || listedCount < count)) {
      const char *name = file.name();
      bool isDirectory = file.isDirectory();
      
      // Apply file type filter
      bool include = true;
      if (fileTypeFilter == 1 && isDirectory) include = false;  // Skip directories
      if (fileTypeFilter == 2 && !isDirectory) include = false; // Skip files
      
      // Apply FileType filter if specified and it's a file
      if (include && fileType != FILETYPE_UNKNOWN && !isDirectory) {
          FileType ft = GetFileTypeFromName(name);
          include = (ft == fileType);
      }

      if (include) {
          if (currentIndex >= startIndex) {
              size_t nameLen = strlen(name);
              size_t requiredLen = currentLen + nameLen + 2;  // +1 newline, +1 null

              if (requiredLen >= outputSize) {
                  file.close();
                  break;  // Buffer full
              }
              
              if (currentLen > 0) {
                  strcat(output, "\n");
                  currentLen++;
              }
              
              strcat(output, name);
              currentLen += nameLen;
              listedCount++;
          }
          currentIndex++;
      }
      file.close();
  }
  
  root.close();
}
// Get file type by filename
FileType GetFileTypeFromName(const char* filename) {
    return GetFileType(getFileExtension(filename));
}




