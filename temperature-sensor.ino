/*
 * ESP32 DHT11 Temperature Sensor
 * This sketch reads temperature and humidity data from a DHT11 sensor
 * and outputs the readings to the serial monitor.
 */

#include "DHT.h"

// Define DHT sensor parameters
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("ESP32 DHT11 Temperature Sensor");
  
  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements
  delay(2000);

  // Reading temperature and humidity takes about 250 milliseconds
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Read temperature in Celsius

  // Check if any reads failed and exit early (to try again)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print humidity and temperature values
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("% | Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
}