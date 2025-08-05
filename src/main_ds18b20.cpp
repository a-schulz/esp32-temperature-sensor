#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

// Arrays to hold device address
DeviceAddress thermometer;

void setup()
{
  Serial.begin(115200); // Initialize serial monitor
  // Remove the pinMode line - OneWire library handles this
  
  // Start up the Dallas Temperature library
  sensors.begin();
  
  // Locate devices on the bus
  Serial.println("Locating devices...");
  Serial.print("Found ");
  int deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount);
  Serial.println(" devices.");

  // Report parasite power mode
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  // Attempt to get address of first device
  if (!sensors.getAddress(thermometer, 0)) {
    Serial.println("Unable to find address for Device 0");
  } else {
    Serial.print("Device 0 Address: ");
    for (uint8_t i = 0; i < 8; i++) {
      if (thermometer[i] < 16) Serial.print("0");
      Serial.print(thermometer[i], HEX);
    }
    Serial.println();
  }
}

void loop()
{
  Serial.println("Requesting temperatures...");
  sensors.requestTemperatures();
  
  // Print temperature for each device found
  for (int i = 0; i < sensors.getDeviceCount(); i++) {
    float tempC = sensors.getTempCByIndex(i);
    if (tempC != DEVICE_DISCONNECTED_C) {
      Serial.print("Device ");
      Serial.print(i);
      Serial.print(" Temperature: ");
      Serial.print(tempC);
      Serial.println(" °C");
    } else {
      Serial.print("Device ");
      Serial.print(i);
      Serial.println(" error: Could not read temperature");
    }
  }
  
  Serial.println("-----------------------------------");
  delay(5000); // Wait 5 seconds between readings
}