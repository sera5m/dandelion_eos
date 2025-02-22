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

std::string getFileExtension(const char *filename) {
    std::string name(filename);
    size_t dotIndex = name.rfind('.');
    if (dotIndex == std::string::npos) return ""; // No extension
    return name.substr(dotIndex + 1);
}

  //known file types
  //text: csv,txt, [not supported yet] doc extensions or anything else
  //audio: mp3,ogg,wav,
  //video: mp4,gif,avi
  //images: png,jpg,bitmap,ico

// Known file types
enum fileType {
    unknown,
    document,
    executable,
    enterperatable, // Custom form of pseudo executable file
    audio,
    video,
    config,
    image
};

// Function to get the file type based on its extension
fileType GetFileType(const std::string& Extension) {
    // Convert the extension to lowercase for case-insensitive comparison
    std::string ext = Extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Map of file extensions to file types
    static const std::unordered_map<std::string, fileType> extensionMap = { //should map file types to my file types enum thingy idk. 
        {"csv", document},
        {"txt", document},
        // {"doc", document}, // Not supported yet
        {"mp3", audio},
        {"ogg", audio},
        {"wav", audio},
        {"mp4", video},
        {"gif", video},
        {"avi", video},
        {"png", image},
        {"jpg", image},
        {"bmp", image},
        {"ico", image},
        {"TDPF", audio}, // data point file ascociated with signal replication in sensors
        {"cmer", enterperatable}, // Mercury file
        {"cfg", config} // Plaintext configs
    };

    // Find and return the file type
    auto it = extensionMap.find(ext);
    if (it != extensionMap.end()) {
        return it->second;
    }

    // Return unknown if not found
    return unknown;
}

//if you want to call this to get the extension of a file, just call GetFileType(getFileExtension(filename));


/*
//the shitty library doesn't even include metadata handling? wow good job expressif. i hate you so much
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
*/

enum DataFlavor{
  analog,
  digital
}

//way to record the datapoints with some custom extensions

//ADPF components

struct TimedDataPoint{ 
    uint16_t value;     // ADC reading (0-4095)--may be treated as raw 
    uint16_t HoldTime; //ANOLOGUE USE: duration that this datapoint stays up in microseconds.      DIGITAL USE: wait x frequency cycles till you do the next datapoint. 

}; __attribute__((packed));//if holdtime is 0, whatever software you're using to enterperet the datapoint defaults holdtime=0 to the frequency


//header for custom file. stores data from sensors in the following format. snapshots of anologue data at a certain frequency.
//think of this like a wav file for arbitrary sensor data for the radio's and rf and such


struct TDPFHeader{ 
double frequency; //any frequency you want, really only used for digital data storage
DataFlavor dataFlavor = DataFlavor::digital; //the flavor of data this file stores. changes how it's enterpereted.
uint64_t creationTime; //metadata for creation time,
//uint8_t bitDepth; //todo: impliment bit depth in the actual datastorage. important only for digital because for anal log 0x0000002 is just 2, but for digital it's very different. probably
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
//use: the file should contain the anolog data points with amplitude and time. the value for timedDataPoint should be a nonzero value on input 

//use cases: the file can be used for playing back any form of datapoints digital or anologue. computers use hex, so a number and a value are the exact same
//when using anologue type, the file just treats the value as the intensity, and outputs a signal for that len of time to an dac circut.
//for digital filetype, the 

//playback should be handled in each component that requires it
//example:
//pin: [digital: plays digital data on a certain frequency, eg value 1=0x3f, value 2=0x4f, frequency is 10hz, so the device plays each of them for 0.1 seconds. ]
//pin: [anolog: plays datapoints into that pin or device. ]

//---------------------------------------------------------------------------------------------------------------------------------------------------------



//streaming the file: just go one after the other with a delay of either tick or holdtime



// Define buffer size (how many data points per batch write)


//MORE HEADERS for the rf and radio

struct RFSettings { //metadata for rf transmisssion
  double carrierFrequency;
  double bitRate;
  double rxBandwidth;
  double freqDeviation;
  double outputPower;
  uint16_t syncWord;
} __attribute__((packed));



// Header including RF settings
struct RTDPFHeader {  //not for general TDPF, this is for
  double frequency;
  DataFlavor dataFlavor = DataFlavor::digital;
  uint64_t creationTime;
  RFSettings rfSettings;
} __attribute__((packed));


//todo: find a way to optionally put this into the TDPF, where this is added along with the header. you'll also have to have the file name be "radio timed datapoint file". 
//effectively should store the same data format becase of whatever. 
//i'd consider just storing the raw data as numbers or whatever, but i don't think there's one best way to do this. to my knoledge the cc1101 radiofrequency would be a lowwer bit width than 16. which is a big difference with the amount of data it stores...



//todo CRITICAL: note: this is an inneficient way to store datapackets, TDPF should only be used for stuff you don't understand, osciliscope data, or anything that doesn't fit a KNOWN type of packet info


#define STREAM_WRITER_BUFFER_SIZE 50
#define STREAM_READER_BUFFER_SIZE 32

class TimedDataPointStreamWriter {
private:
    fs::FS &fs;
    String filePath;
    TimedDataPoint buffer1[STREAM_WRITER_BUFFER_SIZE];  
    TimedDataPoint buffer2[STREAM_WRITER_BUFFER_SIZE];  
    TimedDataPoint* activeBuffer;
    TimedDataPoint* writeBuffer;
    int bufferIndex = 0;  
    bool writingInProgress = false;
    bool headerWritten = false;

    void swapBuffers() {
        TimedDataPoint* temp = activeBuffer;
        activeBuffer = writeBuffer;
        writeBuffer = temp;
        bufferIndex = 0;  
    }

    void writeBufferToFile() {
        if (writingInProgress) return;
        writingInProgress = true;

        File file = fs.open(filePath, FILE_APPEND);
        if (!file) {
            Serial.println("Failed to open file for appending.");
            writingInProgress = false;
            return;
        }
        file.write((uint8_t*)writeBuffer, sizeof(TimedDataPoint) * STREAM_WRITER_BUFFER_SIZE);
        file.close();

        writingInProgress = false;
    }

public:
    TimedDataPointStreamWriter(fs::FS &fs, const String& path) : fs(fs), filePath(path) {
        activeBuffer = buffer1;
        writeBuffer = buffer2;
    }

    bool createFile(TDPFHeader header) {
        File file = fs.open(filePath, FILE_WRITE);
        if (!file) return false;

        file.write((uint8_t*)&header, sizeof(TDPFHeader));
        file.close();
        headerWritten = true;
        return true;
    }

    void writeData(TimedDataPoint newData) {
        if (!headerWritten) {
            Serial.println("Error: Header must be written before data.");
            return;
        }
        activeBuffer[bufferIndex++] = newData;
        if (bufferIndex >= STREAM_WRITER_BUFFER_SIZE) {
            swapBuffers();
            writeBufferToFile();  
        }
    }

    void flush() {
        if (bufferIndex > 0) {
            File file = fs.open(filePath, FILE_APPEND);
            if (file) {
                file.write((uint8_t*)activeBuffer, sizeof(TimedDataPoint) * bufferIndex);
                file.close();
            }
            bufferIndex = 0;
        }
    }
};

class StreamReader {
private:
    fs::FS &fs;
    String filePath;
    File file;
    TDPFHeader header;
    TimedDataPoint buffer[STREAM_READER_BUFFER_SIZE];
    size_t numEntries = 0;
    bool headerRead = false;

public:
    StreamReader(fs::FS &fs, const String& path) : fs(fs), filePath(path) {}

    bool open() {
        file = fs.open(filePath, FILE_READ);
        if (!file) return false;

        if (file.read((uint8_t*)&header, sizeof(TDPFHeader)) == sizeof(TDPFHeader)) {
            headerRead = true;
        }
        return headerRead;
    }

    void close() {
        if (file) file.close();
    }

    bool readNextBlock() {
        if (!file || !file.available()) return false;

        size_t bytesRead = file.read((uint8_t*)buffer, sizeof(TimedDataPoint) * STREAM_READER_BUFFER_SIZE);
        numEntries = bytesRead / sizeof(TimedDataPoint);
        return numEntries > 0;
    }

    TimedDataPoint* getBuffer() {
        return buffer;
    }

    size_t getNumEntries() {
        return numEntries;
    }

    TDPFHeader getHeader() {
        return header;
    }
};

// Example usage
/*
StreamReader reader(SD, "/datafile.tdp");
if (reader.open()) {
    Serial.printf("Frequency: %.2f, Flavor: %d\n", reader.getHeader().frequency, reader.getHeader().dataFlavor);
    while (reader.readNextBlock()) {
        TimedDataPoint* data = reader.getBuffer();
        size_t count = reader.getNumEntries();
        for (size_t i = 0; i < count; i++) {
            Serial.printf("Value: %d, HoldTime: %d\n", data[i].value, data[i].holdTime);
        }
    }
    reader.close();
}
*/


#endif

