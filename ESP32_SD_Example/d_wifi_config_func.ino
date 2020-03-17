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
