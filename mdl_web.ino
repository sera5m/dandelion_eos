#ifndef mdl_web_ino
#define mdl_web_ino
/*
//todo: add watchdog timer and panic handler!
//needs to see reset type
//needs osproscess compat

struct WebsiteConfig {
    String websiteName;
    String filePath;    // Path to HTML file (local or NVS)
    unsigned long updateRateMs;  // Update rate in milliseconds
    String otherRelevantInfo;  // Other config details as needed
};



struct WebsiteConfig {
    String websiteName;
    String filePath;    // Path to HTML file (local or NVS)
    unsigned long updateRateMs;  // Update rate in milliseconds
    String otherRelevantInfo;  // Other config details as needed
};

#include <WiFi.h>
#include <WebServer.h>

// Global vector to store references to active websites
std::vector<WebServer*> activeWebsites;

// Function to launch a web portal using the WebsiteConfig
void launchWebPortal(const WebsiteConfig &config) {
    // Step 1: Activate wireless
    WiFi.begin("YourSSID", "YourPassword");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Step 2: Start the website from the HTML file at the target path
    WebServer* server = new WebServer(80);
    
    // Serve the HTML file (from SPIFFS, NVS, or SD card)
    if (SPIFFS.begin()) {
        server->on("/", HTTP_GET, [&]() {
            File file = SPIFFS.open(config.filePath, "r");
            server->streamFile(file, "text/html");
            file.close();
        });
    }

    // Step 3: Add the website reference to the global vector
    activeWebsites.push_back(server);
    server->begin();

    // Step 4: Ticking update
    unsigned long lastUpdate = millis();
    while (true) {
        if (millis() - lastUpdate >= config.updateRateMs) {
            lastUpdate = millis();
            server->handleClient();  // Keep the website serving
        }
    }
}


// Global vector to store references to active websites
std::vector<WebServer*> activeWebsites;

// Function to launch a web portal using the WebsiteConfig
void launchWebPortal(const WebsiteConfig &config) {
    // Step 1: Activate wireless
    WiFi.begin("YourSSID", "YourPassword");

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Step 2: Start the website from the HTML file at the target path
    WebServer* server = new WebServer(80);
    
    // Serve the HTML file (from SPIFFS, NVS, or SD card)
    if (SPIFFS.begin()) {
        server->on("/", HTTP_GET, [&]() {
            File file = SPIFFS.open(config.filePath, "r");
            server->streamFile(file, "text/html");
            file.close();
        });
    }

    // Step 3: Add the website reference to the global vector
    activeWebsites.push_back(server);
    server->begin();

    // Step 4: Ticking update
    unsigned long lastUpdate = millis();
    while (true) {
        if (millis() - lastUpdate >= config.updateRateMs) {
            lastUpdate = millis();
            server->handleClient();  // Keep the website serving
        }
    }
}


class WebsiteManagerService {
private:
    bool websiteActive = false;

public:
    void startService() {
        // Only start service if no website is online
        if (!websiteActive) {
            websiteActive = true;
            Serial.println("Starting website manager service...");
            // Start website functionality, e.g., web portal
            WebsiteConfig config;
            config.websiteName = "MyWebsite";
            config.filePath = "/index.html";
            config.updateRateMs = 500;
            launchWebPortal(config);
        }
    }

    void stopService() {
        // Stop all websites and turn off WiFi if no websites online
        if (websiteActive) {
            websiteActive = false;
            for (auto& server : activeWebsites) {
                server->stop();
            }
            WiFi.disconnect();
            Serial.println("No active websites, turning off WiFi.");
        }
    }

    void update() {
        // Handle periodic updates for the website
        for (auto& server : activeWebsites) {
            server->handleClient(); // Ensure clients are served
        }
    }
};


//useguide-MAY BE OUT OF DATE 3/10/25

vWebsiteManagerService websiteService;

void setup() {
    Serial.begin(115200);
if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
}
Serial.println("SPIFFS initialized.");
//or do sp sd.begin or whatever

    // Start the website manager service
    websiteService.startService();
}

void loop() {
    // Periodically update websites (if active)
    websiteService.update();
    delay(100);
}

*/


#endif


