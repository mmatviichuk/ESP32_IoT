void setup() {
  #if (DEBUG_SERIAL == 1)
  Serial.begin(115200);
  DEBUG("start");
  #endif


  pinMode(ledPin, OUTPUT);

  delay(100);

  if (!SD.begin(SD_CS)) {
    Serial.println("");
    DEBUG("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    DEBUG("");
    return;
  }

  Serial.print("SD Card Type: ");
  DEBUG("");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
    DEBUG("");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
    DEBUG("");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
    DEBUG("");
  } else {
    Serial.println("UNKNOWN");
    DEBUG("");
  }

  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  DEBUG("");
  loadWiFiCoaanfiguration(filename, config);

  //connect to WiFi
  Serial.printf("Connecting to %s ", config.ssid);
  DEBUG("");
  WiFi.begin(config.ssid, config.pswd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    DEBUG("");
  }
  Serial.println(" CONNECTED");
  DEBUG("");

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  DEBUG("");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  File file = SD.open("/data.txt");
  if (!file) {
    Serial.println("File doens't exist");
    DEBUG("");
    Serial.println("Creating file...");
    DEBUG("");
    writeFile(SD, "/data.txt", "Date\tTime\tTemperature\tHumidity\r\n");
  }
  else {
    Serial.println("File already exists");
    DEBUG("");
  }
  file.close();

  pinMode(DHTPin, INPUT);
  dht.begin();

  getSensorData();
  getTime();
  payloadMessage();

  //listDir(SD, "/", 2);

  appendFile(SD, "/data.txt", dataMessage.c_str());
  //readFile(SD, "/data.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  DEBUG("");
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  DEBUG("");/**/

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}
