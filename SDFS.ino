#ifndef SDFS_H
#define SDFS_h
#include "Wiring.h"

/*
 * pin 1 - not used          |  Micro SD card     |
 * pin 2 -    (SS)           |                   /
 * pin 3 -    (MOSI)         |                  |__
 * pin 4 -     (3.3V)        |                    |
 * pin 5 -     (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 -     (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 -     (MISO)        | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - not connected     |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Connections for     ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * full-sized          ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * SD card             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Pin name         |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pin number    |                                    /
 *                  |                                  █/
 *                  |__▍___▊___█___█___█___█___█___█___/
 *
*/
/*
void initSDCard() {
    spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS_SD);
    
    if (!SD.begin(SPI_CS_SD, spi, 4000000)) {  // 4MHz speed
        Serial.println("Card Mount Failed");
        return;
    }
    
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }
}
*/


//disabled most stuff for now sadly




/*============================================================
__________________
|    _____________|__
|   /  file system  /
|  /   advanced    / 
| /   components  /
|/_______________/
*///=========================================================

/*

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

*/

#endif

