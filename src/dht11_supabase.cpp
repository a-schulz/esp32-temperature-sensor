#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <ESPSupabase.h>
#include "credentials.h" // Include credentials header

// Define DHT sensor parameters
#define DHTPIN 4     // Digital pin connected to DHT sensor
#define DHTTYPE DHT11

// Initialize objects
DHT dht(DHTPIN, DHTTYPE);
Supabase supabase;
unsigned long lastMsg = 0;

// Configuration - change this to your desired location
const char* LOCATION = "food_storage"; // Compile-time location setting

void setup_wifi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Use credentials from header
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendToSupabase(const char* type, float value) {
  String tableName = "environment_measurements"; // Change to your table name
  
  // Create JSON data with your table structure
  String jsonData = "{\"location\": \"" + String(LOCATION) + "\", \"type\": \"" + String(type) + "\", \"value\": " + String(value, 2) + "}";
  
  Serial.print("Sending to Supabase: ");
  Serial.println(jsonData);
  
  int response = supabase.insert(tableName, jsonData, false);
  if (response == 200 || response == 201) {
    Serial.println("Data inserted successfully!");
  } else {
    Serial.print("Failed to insert data. HTTP response: ");
    Serial.println(response);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  
  // Initialize Supabase
  supabase.begin(SUPABASE_URL, SUPABASE_KEY); // Use credentials from header
  
  Serial.println("Setup complete. Starting sensor readings...");
}

void loop() {
  unsigned long now = millis();
  if (now - lastMsg > 60000) {  // Send every 60 seconds to avoid too frequent inserts
    lastMsg = now;
    
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Send temperature reading
    sendToSupabase("temperature", temperature);
    delay(1000); // Small delay between requests
    
    // Send humidity reading
    sendToSupabase("humidity", humidity);

    // Print to serial for debugging
    Serial.print("Readings sent - Temperature: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }
  
  delay(1000); // Small delay in loop
}