//PIN Definition
#define DHTTYPE DHT11   // DHT 11
#define SD_CS 5
////////////////

//Configuration 
#define DEBUG_SERIAL 1 
#define REPEAT_CHECK 20000

//Include section
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "DHT.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "time.h"
#include "HTTPClient.h"

//WiFi Config variables
const char* filename = "/wifi.ini";
struct Config {
  char ssid[64];
  char pswd[64];
};
Config config;

//Real time server configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 14400;
const int   daylightOffset_sec = 3600;
char Date [10];
char Time [10];

//Variables
int ledPin = 2;
//
uint8_t DHTPin = 4;
float Temperature;
float Humidity;
DHT dht(DHTPin, DHTTYPE);
//
String dataMessage;

#if (DEBUG_SERIAL == 1)
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
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
        listDir(fs, file.name(), levels - 1);
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
