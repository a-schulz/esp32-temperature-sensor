#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 4     // GPIO pin connected to the DHT11 sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    dht.begin();
}

void loop() {
    delay(2000); // Wait a few seconds between measurements

    float h = dht.readHumidity();    // Read humidity
    float t = dht.readTemperature();  // Read temperature in Celsius

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Print the results to the Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
}