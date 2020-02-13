void getSensorData() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  Serial.print("Temperature: "); Serial.print(Temperature); Serial.print(" Humidity: "); Serial.println(Humidity);
  DEBUG("");
}
