void loop() {
  delay(5000);
  getSensorData();
  getTime();
  payloadMessage();
  appendFile(SD, "/data.txt", dataMessage.c_str());
}
