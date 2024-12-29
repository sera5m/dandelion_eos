#ifndef fileSystem_ino
#define fileSystem_ino

#include "FS.h" //WHY DO I HAVE TOD EFINE IT HERE IT MAKES NO SENSE
/*
 * pin 1 - not used          |  Micro SD card     |
 * pin 2 - CS (SS)           |                   /
 * pin 3 - DI (MOSI)         |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - SCK (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 - DO (MISO)         | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - not used          |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Connections for     ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * full-sized          ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * SD card             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Pin name         |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pin number    |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  █/
 *                  |__▍___▊___█___█___█___█___█___█___/
 *
 * Note:  The SPI pins can be manually configured by using `SPI.begin(sck, miso, mosi, cs);`.
 *        Alternatively you can change only the CS pin with `SD.begin(CSpin)` and keep the default settings for other pins.
 *
+----------+
 * | SPI Pin Name | ESP8266 | ESP32 | ESP32‑S2 | ESP32‑S3 | ESP32‑C3 | ESP32‑C6 | ESP32‑H2 |
 * +==============+=========+=======+==========+==========+==========+==========+==========+
 * | CS (SS)      | GPIO15  | GPIO5 | GPIO34   | GPIO10   | GPIO7    | GPIO18   | GPIO0    |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DI (MOSI)    | GPIO13  | GPIO23| GPIO35   | GPIO11   | GPIO6    | GPIO19   | GPIO25   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DO (MISO)    | GPIO12  | GPIO19| GPIO37   | GPIO13   | GPIO5    | GPIO20   | GPIO11   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SCK (SCLK)   | GPIO14  | GPIO18| GPIO36   | GPIO12   | GPIO4    | GPIO21   | GPIO10   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 *
 * For more info see file README.md in this library or on URL:
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 */

//okay time to copy paste default shit

//okay to use this, call functions like this
//list,create
/*

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
 
  removeDir(SD, "/mydir");
  listDir(SD, "/", 2);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

*/

//TODO: make a file header recognition system for txt, program files,csv, pnj/jpg/gif/etc becasue this doesn't really work.
// seperate by use because text files are different uses than anything else. 

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

void moveFile(fs::FS &fs, const char *source, const char *destination) {
  Serial.printf("Moving file from %s to %s\n", source, destination);
  if (fs.rename(source, destination)) {
    Serial.println("File moved successfully");
  } else {
    Serial.println("Move failed");
  }
}

void copyFile(fs::FS &fs, const char *srcPath, const char *destPath) {
  Serial.printf("Copying file from %s to %s\n", srcPath, destPath);

  // Open the source file
  File srcFile = fs.open(srcPath, FILE_READ);
  if (!srcFile) {
    Serial.println("Failed to open source file");
    return;
  }

  // Open the destination file
  File destFile = fs.open(destPath, FILE_WRITE);
  if (!destFile) {
    Serial.println("Failed to open destination file");
    srcFile.close();
    return;
  }

  // Read from the source file and write to the destination file
  uint8_t buffer[512]; // Buffer to hold file chunks
  size_t bytesRead;
  while ((bytesRead = srcFile.read(buffer, sizeof(buffer))) > 0) {
    destFile.write(buffer, bytesRead);
  }

  Serial.println("File copied successfully");

  // Close files
  srcFile.close();
  destFile.close();
}

String getFileExtension(const char *filename) {
  String name = String(filename);
  int dotIndex = name.lastIndexOf('.');
  if (dotIndex == -1) return ""; // No extension
  return name.substring(dotIndex + 1);
}

///*
void getFileMetadata(fs::FS &fs, const char *path) {
  Serial.printf("Fetching metadata for: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file");
    return;
  }

  Serial.printf("File Name: %s\n", file.name());
  Serial.printf("File Size: %u bytes\n", file.size());

  // If using SDFat, you can access timestamps
  uint64_t creationTime = file.getCreationTime();
  uint64_t lastWriteTime = file.getLastWrite();

  if (creationTime > 0 && lastWriteTime > 0) {
    Serial.printf("Creation Time: %u\n", creationTime);
    Serial.printf("Last Modified: %u\n", lastWriteTime);
  } else {
    Serial.println("No timestamp information available");
  }

  file.close();
}

void writeFileMetadata(fs::FS &fs, const char *path, const char *key, const char *value) { //does this need to be fixed
  String metaFilePath = String(path) + ".meta"; // Example: "/file.txt.meta"

  Serial.printf("Writing metadata for: %s\n", path);

  File metaFile = fs.open(metaFilePath.c_str(), FILE_WRITE);
  if (!metaFile) {
    Serial.println("Failed to open metadata file");
    return;
  }

  metaFile.printf("%s: %s\n", key, value);
  metaFile.close();

  Serial.println("Metadata written successfully");
}
//*/

#endif