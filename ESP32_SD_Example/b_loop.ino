void loop() {
  delay(REPEAT_CHECK);
  getSensorData();
  getTime();
  payloadMessage();
  // check if file exist -> create file or append file
  appendFile(SD, "/data.txt", dataMessage.c_str());

  wifiConnect(config);
  //post message to site
  postData();
  wifiDisconnect();
}
