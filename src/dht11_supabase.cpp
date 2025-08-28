#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <ESPSupabase.h>
#include "credentials.h" // Include credentials header

// Define DHT sensor parameters
#define DHTPIN 4     // Digital pin connected to DHT sensor
#define DHTTYPE DHT11

// Deep sleep configuration
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  900         // Time ESP32 will go to sleep (in seconds) - 30 seconds

// Initialize objects
DHT dht(DHTPIN, DHTTYPE);
Supabase supabase;

// Configuration - change this to your desired location
const char* LOCATION = "food_storage"; // Compile-time location setting

// RTC memory to store boot count (survives deep sleep)
RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup_wifi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Timeout after 30 seconds if can't connect
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void sendToSupabase(const char* type, float value) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping Supabase upload");
    return;
  }
  
  String tableName = "environment_measurements";
  
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
  delay(1000); // Give serial time to initialize
  
  // Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  
  // Print the wakeup reason for ESP32
  print_wakeup_reason();
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  setup_wifi();
  
  // Initialize Supabase only if WiFi connected
  if (WiFi.status() == WL_CONNECTED) {
    supabase.begin(SUPABASE_URL, SUPABASE_KEY);
  }
  
  // Read sensor data
  Serial.println("Reading sensor data...");
  
  // Wait a bit for DHT sensor to stabilize
  delay(2000);
  
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Print readings
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    
    // Send data to Supabase if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
      sendToSupabase("temperature", temperature);
      delay(1000); // Small delay between requests
      sendToSupabase("humidity", humidity);
      
      // Wait a bit for data to be sent
      delay(2000);
    }
  }
  
  // Configure the timer to wake us up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP) + " seconds");
  
  // Disconnect WiFi to save power
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  Serial.println("Going to sleep now");
  Serial.flush(); // Make sure all serial output is sent
  
  // Go to deep sleep
  esp_deep_sleep_start();
}

void loop() {
  // This will never be reached because we go to deep sleep in setup()
  // Deep sleep resets the ESP32, so it starts from setup() again
}