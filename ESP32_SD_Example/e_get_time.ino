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
