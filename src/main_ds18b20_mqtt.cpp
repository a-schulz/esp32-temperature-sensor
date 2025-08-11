#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "credentials.h" // Include credentials header

#define ONE_WIRE_BUS 8
#define SDA_PIN 5
#define SCL_PIN 6

// Device identification
const char* device_id = "heating_sensor_01";
const char* location = "boiler_room"; // Change for each sensor location

// MQTT topics
String temp_topic = "heating/" + String(location) + "/temperature";
String status_topic = "heating/" + String(location) + "/status";
String discovery_topic = "homeassistant/sensor/" + String(device_id) + "/config";

WiFiClient espClient;
PubSubClient client(espClient);
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long lastMsg = 0;

// Forward declarations
void setup_wifi();
void reconnect();
void publishTemperature(float temp);
void publishStatus(String status, float temp);
void sendDiscoveryConfig();
void showTemperature(float temp);
void showStatus(String message);
void showError();

void setup_wifi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Use credentials from header
  showStatus("WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    showStatus("MQTT...");
    
    String clientId = "ESP32-" + String(device_id);
    
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) { // Use credentials from header
      Serial.println("MQTT connected");
      publishStatus("online", -999);
      sendDiscoveryConfig();
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  showStatus("Starting...");
  
  // Initialize temperature sensor
  sensors.begin();
  
  // Connect to WiFi
  setup_wifi();
  
  // Setup MQTT
  client.setServer(MQTT_SERVER, MQTT_PORT); // Use credentials from header
  
  showStatus("Ready");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long now = millis();
  if (now - lastMsg > 30000) {  // Publish every 30 seconds
    lastMsg = now;
    
    // Read temperature
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    
    if (temperature != DEVICE_DISCONNECTED_C) {
      // Publish to MQTT
      publishTemperature(temperature);
      
      // Update display
      showTemperature(temperature);
      
      // Publish status
      publishStatus("online", temperature);
      
      Serial.print("Published - Temperature: ");
      Serial.print(temperature);
      Serial.println("°C");
    } else {
      showError();
      publishStatus("sensor_error", -999);
      Serial.println("Failed to read from DS18B20 sensor!");
    }
  }
}

void publishTemperature(float temp) {
  char tempString[8];
  dtostrf(temp, 1, 1, tempString);
  client.publish(temp_topic.c_str(), tempString, true);
}

void publishStatus(String status, float temp) {
  StaticJsonDocument<200> doc;
  doc["device_id"] = device_id;
  doc["location"] = location;
  doc["status"] = status;
  doc["temperature"] = temp;
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  
  char payload[256];
  serializeJson(doc, payload);
  client.publish(status_topic.c_str(), payload, true);
}

void sendDiscoveryConfig() {
  StaticJsonDocument<500> doc;
  doc["name"] = String(location) + " Temperature";
  doc["device_class"] = "temperature";
  doc["state_topic"] = temp_topic;
  doc["unit_of_measurement"] = "°C";
  doc["unique_id"] = device_id;
  
  JsonObject device = doc.createNestedObject("device");
  device["identifiers"][0] = device_id;
  device["name"] = "Heating Sensor " + String(location);
  device["model"] = "ESP32-C3 DS18B20";
  device["manufacturer"] = "Custom";
  
  char payload[512];
  serializeJson(doc, payload);
  client.publish(discovery_topic.c_str(), payload, true);
}

void showTemperature(float temp) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso18_tf);
  
  char tempStr[10];
  sprintf(tempStr, "%.1f", temp);
  u8g2.drawStr(0, 25, tempStr);
  
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(45, 35, "C");
  
  // Show connection status
  if (WiFi.status() == WL_CONNECTED) {
    u8g2.drawStr(60, 10, "W");
  }
  
  if (client.connected()) {
    u8g2.drawStr(65, 10, "M");
  }
  
  u8g2.sendBuffer();
}

void showStatus(String message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 20, message.c_str());
  u8g2.sendBuffer();
}

void showError() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 15, "SENSOR");
  u8g2.drawStr(0, 25, "ERROR!");
  u8g2.sendBuffer();
}