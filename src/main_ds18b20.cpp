#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>
#include <Wire.h>

#define ONE_WIRE_BUS 8
#define SDA_PIN 5
#define SCL_PIN 6

// Setup U8g2 display object for your specific OLED
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

// Arrays to hold device address
DeviceAddress thermometer;

void setup()
{
  Serial.begin(115200);
  
  // Initialize the OLED display
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  
  // Display initialization message
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Temperature");
  u8g2.drawStr(0, 20, "Sensor");
  u8g2.drawStr(0, 30, "Starting...");
  u8g2.sendBuffer();
  
  // Start up the Dallas Temperature library
  sensors.begin();
  
  // Locate devices on the bus
  Serial.println("Locating devices...");
  int deviceCount = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(deviceCount);
  Serial.println(" devices.");
  
  // Update display with device count
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "DS18B20 Sensor");
  
  char deviceText[20];
  sprintf(deviceText, "Found: %d", deviceCount);
  u8g2.drawStr(0, 20, deviceText);
  
  if (deviceCount == 0) {
    u8g2.drawStr(0, 30, "No sensor!");
    u8g2.drawStr(0, 40, "Check wiring");
  }
  
  u8g2.sendBuffer();
  delay(3000);

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
  
  // Clear display for new reading
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Get device count
  int deviceCount = sensors.getDeviceCount();
  
  if (deviceCount == 0) {
    u8g2.drawStr(0, 10, "No sensors!");
    u8g2.drawStr(0, 20, "Check GPIO4");
    u8g2.drawStr(0, 30, "4.7k pullup");
  } else {
    // Print temperature for each device found
    for (int i = 0; i < deviceCount; i++) {
      float tempC = sensors.getTempCByIndex(i);
      
      Serial.print("Device ");
      Serial.print(i);
      
      if (tempC != DEVICE_DISCONNECTED_C) {
        Serial.print(" Temperature: ");
        Serial.print(tempC);
        Serial.println(" °C");
        
        // Display on OLED
        char sensorText[20];
        sprintf(sensorText, "Sensor %d:", i);
        u8g2.drawStr(0, 10, sensorText);
        
        char tempText[20];
        sprintf(tempText, "%.1f C", tempC);
        u8g2.drawStr(0, 20, tempText);
        
        // Convert to Fahrenheit and display
        float tempF = tempC * 9.0 / 5.0 + 32.0;
        char tempFText[20];
        sprintf(tempFText, "%.1f F", tempF);
        u8g2.drawStr(0, 30, tempFText);
        
      } else {
        Serial.println(" error: Could not read temperature");
        
        char sensorText[20];
        sprintf(sensorText, "Sensor %d:", i);
        u8g2.drawStr(0, 10, sensorText);
        u8g2.drawStr(0, 20, "ERROR!");
        u8g2.drawStr(0, 30, "Check wire");
      }
    }
  }
  
  // Add uptime
  char uptimeText[20];
  sprintf(uptimeText, "Up: %lus", millis() / 1000);
  u8g2.drawStr(0, 40, uptimeText);
  
  // Update display
  u8g2.sendBuffer();
  
  Serial.println("-----------------------------------");
  delay(1000); // Wait x/1000 seconds between readings
}