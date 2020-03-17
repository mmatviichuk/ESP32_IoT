void loop() {
  delay(REPEAT_CHECK);
  getSensorData();
  getTime();
  payloadMessage();
  appendFile(SD, "/data.txt", dataMessage.c_str());
}
