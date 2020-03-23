///////////////////////////////////////////////////
/////SD_CARD functions/////////////////////////////
///////////////////////////////////////////////////
void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path) {
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

void writeFile(fs::FS &fs, const char * path, const char * message) {
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

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);
  digitalWrite(ledPin, HIGH);
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    digitalWrite(ledPin, LOW);
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
    digitalWrite(ledPin, LOW);
  } else {
    Serial.println("Append failed");
    digitalWrite(ledPin, LOW);
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path) {
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
    Serial.printf("%u bytes read for %u ms\n", flen, end);
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
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

///////////////////////////////////////////////////
/////WiFi Functions////////////////////////////////
///////////////////////////////////////////////////
void loadWiFiCoaanfiguration(const char *filename, Config &config) {

  File file = SD.open(filename);

  StaticJsonDocument<512> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println("Failed to read file, using default configuration");
    DEBUG("");


  strlcpy(config.ssid, doc["ssid"] | "WIFI_SSID", sizeof(config.ssid));
  strlcpy(config.pswd, doc["pswd"] | "WIFI_PASSWD", sizeof(config.pswd));

  file.close();
}

void wifiConnect (Config &config){
  //connect to WiFi
  //Serial.printf("Connecting to %s ", config.ssid);
  DEBUG("Connecting to "+ String(config.ssid)+" ps:"+String(config.pswd));
  WiFi.begin(config.ssid, config.pswd);
  Serial.print(WiFi.isConnected());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."+WiFi.isConnected());
    DEBUG("");
  }
  Serial.println(" CONNECTED");
  DEBUG("");
}

void wifiDisconnect(){
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
  DEBUG("WiFi disconnected");
}

///////////////////////////////////////////////////
/////Get Time Functions////////////////////////////
///////////////////////////////////////////////////
void getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    DEBUG("");
    return;
  }
  Serial.println(&timeinfo, "%Y%m%d %H:%M:%S");
  DEBUG("");
  strftime (Date, 10, "%Y%m%d", &timeinfo);
  strftime (Time, 10, "%H:%M:%S", &timeinfo);
}

///////////////////////////////////////////////////
/////Get Sensor data///////////////////////////////
///////////////////////////////////////////////////
void getSensorData() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  Serial.print("Temperature: "); Serial.print(Temperature); Serial.print(" Humidity: "); Serial.println(Humidity);
  DEBUG("");
}

///////////////////////////////////////////////////
/////Prepare Payload Message///////////////////////
///////////////////////////////////////////////////
void payloadMessage() {
  dataMessage  = String(Date) + "\t" + String(Time) + "\t" + String(Temperature) + "\t" + String(Humidity) + "\r\n";
  //Serial.print("PayloadMessage:"); Serial.println(dataMessage);
  DEBUG("PayloadMessage: " + dataMessage);
}

///////////////////////////////////////////////////
/////Rest API functions////////////////////////////
///////////////////////////////////////////////////
void postData(){
  DEBUG("postData Start");
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient https;
    Serial.print("[HTTPS] begin...\n");
    //String(Date) + "\t" + String(Time) + "\t" + String(Temperature) + "\t" + String(Humidity) + "\r\n"
    if (https.begin("https://ilogdata.online/wp-json/mm/v1/test/?date=" + String(Date) + "&time=" + String(Time) + "&temp=" + String(Temperature) + "&humid=" + String(Humidity) + "&device=Test_device")) {  // HTTPS
      //#if 0
      String authorization = String("Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJodHRwczpcL1wvaWxvZ2RhdGEub25saW5lIiwiaWF0IjoxNTg0NDYzNzM4LCJuYmYiOjE1ODQ0NjM3MzgsImV4cCI6MTU4NTA2ODUzOCwiZGF0YSI6eyJ1c2VyIjp7ImlkIjoiMiJ9fX0.rL8ZMVZvK4pG1yXd9Fl6JDJWA149wGrbv9Ii9plCE1I");
      Serial.println(String("authorization = ") + authorization);
      https.addHeader("authorization", authorization);
      https.addHeader("content-type", "text/plain");
      https.addHeader("cache-control", "no-cache");
      //#endif
      Serial.print("[HTTPS] POST...\n");
      // start connection and send HTTP header
      int httpCode = https.POST("{\"binary_data\": \"aGVsbG8K\"}");

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          Serial.println(String("see payload") + https.getSize());
          if  (https.getSize() > 0) {
            String payload = https.getString();
            Serial.println(payload);
          }
          Serial.println("result = true");
          //result = true;
        } else Serial.println("result = false");//result = false;
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        String payload = https.getString();
        Serial.println(String("payload:") + payload);
        Serial.println("result = false");//result = false;
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
      Serial.println("result = false");//result = false;
    }

    // End extra scoping block
  }
}
