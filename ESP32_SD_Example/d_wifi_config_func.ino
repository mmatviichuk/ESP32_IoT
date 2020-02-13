// Loads the configuration from a file
void loadWiFiCoaanfiguration(const char *filename, Config &config) {
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
    DEBUG("");

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
