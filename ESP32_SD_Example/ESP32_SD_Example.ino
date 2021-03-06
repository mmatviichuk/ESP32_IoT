#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "DHT.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "time.h"

#define DHTTYPE DHT11   // DHT 11
#define SD_CS 5


struct Config {
  char ssid[64];
  char pswd[64];
};

const char* filename = "/wifi.ini";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 14400;
const int   daylightOffset_sec = 3600;

int ledPin = 2;
float Temperature;
float Humidity;
String dataMessage;
char Time [80];

Config config; 

uint8_t DHTPin = 4;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Loads the configuration from a file
void loadWiFiConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File file = SD.open(filename);

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  strlcpy(config.ssid,                  // <- destination
          doc["ssid"] | "WIFI_SSID",  // <- source
          sizeof(config.ssid));         // <- destination's capacity
  //config.port = doc["port"] | 2731;
  strlcpy(config.pswd,                  // <- destination
          doc["pswd"] | "WIFI_PASSWD",  // <- source
          sizeof(config.pswd));         // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
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

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);
    digitalWrite(ledPin, HIGH);
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        digitalWrite(ledPin, LOW);
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
        digitalWrite(ledPin, LOW);
    } else {
        Serial.println("Append failed");
        digitalWrite(ledPin, LOW);
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void GetTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y%m%d %H:%M:%S");
  strftime (Time,80,"%Y%m%d %H:%M:%S",&timeinfo);
}

void GetSensorData() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  Serial.print("Temperature: "); Serial.print(Temperature); Serial.print(" Humidity: "); Serial.println(Humidity);  
}

void PayloadMessage() {
  dataMessage  = String(Time)+"\t"+String(Temperature) +"\t"+ String(Humidity) +"\r\n";
  Serial.print("PayloadMessage:");Serial.println(dataMessage);
}

void setup(){
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  
  delay(100);
  
  if(!SD.begin(SD_CS)){
      Serial.println("Card Mount Failed");
      return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadWiFiConfiguration(filename, config);

  //connect to WiFi
  Serial.printf("Connecting to %s ", config.ssid);
  WiFi.begin(config.ssid, config.pswd);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "TimeStamp\tTemperature\tHumidity\r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();

  pinMode(DHTPin, INPUT);
  dht.begin();
  
  GetSensorData();
  GetTime();
  PayloadMessage();
  
  //listDir(SD, "/", 2);
    
  appendFile(SD, "/data.txt", dataMessage.c_str());
  //readFile(SD, "/data.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));/**/

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop(){
  delay(5000);
  GetSensorData();
  GetTime();
  PayloadMessage();
  appendFile(SD, "/data.txt", dataMessage.c_str());
}
