#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include "credentials.h" // Include credentials header

// Define DHT sensor parameters
#define DHTPIN 4     // Digital pin connected to DHT sensor
#define DHTTYPE DHT11

// Function prototypes - add these before any function uses them
void handleRoot();
void handleReadings();

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Use credentials from header
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/readings", handleReadings);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(100);
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='5'>"; // Auto refresh page every 5 seconds
  html += "<title>ESP32 Temperature Monitor</title>";
  html += "<style>body{font-family:Arial;text-align:center;margin-top:50px;}</style>";
  html += "</head><body>";
  html += "<h1>ESP32 Temperature Monitor</h1>";
  
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    html += "<p>Failed to read from DHT sensor!</p>";
  } else {
    html += "<h2>Temperature: " + String(temperature) + " °C</h2>";
    html += "<h2>Humidity: " + String(humidity) + " %</h2>";
  }
  
  html += "<p>Last updated: " + String(millis() / 1000) + " seconds ago</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleReadings() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  String json = "{";
  if (isnan(humidity) || isnan(temperature)) {
    json += "\"error\":\"Failed to read from DHT sensor!\"";
  } else {
    json += "\"temperature\":" + String(temperature) + ",";
    json += "\"humidity\":" + String(humidity);
  }
  json += "}";
  
  server.send(200, "application/json", json);
}