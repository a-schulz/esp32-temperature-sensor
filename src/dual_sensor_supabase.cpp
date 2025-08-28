#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <ESPSupabase.h>
#include "credentials.h"

// ========== SENSOR CONFIGURATION ==========
// DHT11 Sensor Configuration
#define DHTPIN 4
#define DHTTYPE DHT11

// DS18B20 Sensor Configuration
#define ONE_WIRE_BUS 8

// Location Configuration - Change these for different deployments
const char* DHT_LOCATION = "garage";      // Location for DHT11 sensor
const char* DS18B20_LOCATION = "heating"; // Location for DS18B20 sensor

// ========== DEEP SLEEP CONFIGURATION ==========
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 900          // Time ESP32 will go to sleep (in seconds) - 15 minutes

// ========== SENSOR OBJECTS ==========
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
Supabase supabase;

// ========== RTC MEMORY (survives deep sleep) ==========
RTC_DATA_ATTR int bootCount = 0;

// ========== HELPER FUNCTIONS ==========
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup: External signal (RTC_IO)");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup: External signal (RTC_CNTL)");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup: Timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup: Touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup: ULP program");
      break;
    default:
      Serial.printf("Wakeup: Not caused by deep sleep (%d)\n", wakeup_reason);
      break;
  }
}

bool setup_wifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Timeout after 30 seconds
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nFailed to connect to WiFi");
    return false;
  }
}

bool sendToSupabase(const char* location, const char* type, float value) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping Supabase upload");
    return false;
  }
  
  String tableName = "environment_measurements";
  String jsonData = "{\"location\": \"" + String(location) + 
                   "\", \"type\": \"" + String(type) + 
                   "\", \"value\": " + String(value, 2) + "}";
  
  Serial.print("Sending to Supabase: ");
  Serial.println(jsonData);
  
  int response = supabase.insert(tableName, jsonData, false);
  if (response == 200 || response == 201) {
    Serial.println("✓ Data inserted successfully!");
    return true;
  } else {
    Serial.print("✗ Failed to insert data. HTTP response: ");
    Serial.println(response);
    return false;
  }
}

struct SensorReadings {
  float dht_temperature = NAN;
  float dht_humidity = NAN;
  float ds18b20_temperature = NAN;
  bool dht_success = false;
  bool ds18b20_success = false;
};

SensorReadings readAllSensors() {
  SensorReadings readings;
  
  Serial.println("=== Reading Sensors ===");
  
  // Read DHT11 sensor
  Serial.println("Reading DHT11...");
  delay(2000); // DHT needs time to stabilize
  
  readings.dht_humidity = dht.readHumidity();
  readings.dht_temperature = dht.readTemperature();
  
  if (isnan(readings.dht_humidity) || isnan(readings.dht_temperature)) {
    Serial.println("✗ DHT11: Failed to read sensor");
    readings.dht_success = false;
  } else {
    Serial.printf("✓ DHT11: %.1f°C, %.1f%% humidity\n", 
                  readings.dht_temperature, readings.dht_humidity);
    readings.dht_success = true;
  }
  
  // Read DS18B20 sensor
  Serial.println("Reading DS18B20...");
  ds18b20.requestTemperatures();
  delay(1000); // Give DS18B20 time to convert
  
  readings.ds18b20_temperature = ds18b20.getTempCByIndex(0);
  
  if (readings.ds18b20_temperature == DEVICE_DISCONNECTED_C || 
      readings.ds18b20_temperature == -127.0) {
    Serial.println("✗ DS18B20: Failed to read sensor");
    readings.ds18b20_success = false;
  } else {
    Serial.printf("✓ DS18B20: %.1f°C\n", readings.ds18b20_temperature);
    readings.ds18b20_success = true;
  }
  
  return readings;
}

bool uploadSensorData(const SensorReadings& readings) {
  bool allSuccess = true;
  
  Serial.println("=== Uploading Data ===");
  
  // Upload DHT11 data
  if (readings.dht_success) {
    if (!sendToSupabase(DHT_LOCATION, "temperature", readings.dht_temperature)) {
      allSuccess = false;
    }
    delay(1000);
    
    if (!sendToSupabase(DHT_LOCATION, "humidity", readings.dht_humidity)) {
      allSuccess = false;
    }
    delay(1000);
  }
  
  // Upload DS18B20 data
  if (readings.ds18b20_success) {
    if (!sendToSupabase(DS18B20_LOCATION, "temperature", readings.ds18b20_temperature)) {
      allSuccess = false;
    }
    delay(1000);
  }
  
  return allSuccess;
}

void printSystemInfo() {
  Serial.println("=== System Information ===");
  Serial.printf("Boot #: %d\n", bootCount);
  Serial.printf("DHT11 Location: %s\n", DHT_LOCATION);
  Serial.printf("DS18B20 Location: %s\n", DS18B20_LOCATION);
  Serial.printf("Sleep Duration: %d seconds\n", TIME_TO_SLEEP);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  print_wakeup_reason();
  Serial.println("========================");
}

void initializeSensors() {
  Serial.println("=== Initializing Sensors ===");
  
  // Initialize DHT11
  Serial.println("Initializing DHT11...");
  dht.begin();
  
  // Initialize DS18B20
  Serial.println("Initializing DS18B20...");
  ds18b20.begin();
  
  // Check DS18B20 device count
  int deviceCount = ds18b20.getDeviceCount();
  Serial.printf("DS18B20 devices found: %d\n", deviceCount);
  
  if (deviceCount == 0) {
    Serial.println("⚠ Warning: No DS18B20 devices found! Check wiring and 4.7kΩ pullup resistor.");
  }
  
  // Report parasite power mode
  Serial.printf("DS18B20 Parasite power: %s\n", 
               ds18b20.isParasitePowerMode() ? "ON" : "OFF");
  
  Serial.println("Sensors initialized!");
}

void enterDeepSleep() {
  Serial.println("=== Preparing for Deep Sleep ===");
  
  // Disconnect WiFi to save power
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // Configure timer wakeup
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  
  Serial.printf("Deep sleep configured for %d seconds\n", TIME_TO_SLEEP);
  Serial.println("Going to sleep now...");
  Serial.flush(); // Ensure all serial output is sent
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  // Increment and display boot count
  ++bootCount;
  
  // Print system information
  printSystemInfo();
  
  // Initialize sensors
  initializeSensors();
  
  // Connect to WiFi
  bool wifiConnected = setup_wifi();
  
  // Initialize Supabase if WiFi connected
  if (wifiConnected) {
    Serial.println("Initializing Supabase connection...");
    supabase.begin(SUPABASE_URL, SUPABASE_KEY);
  }
  
  // Read all sensors
  SensorReadings readings = readAllSensors();
  
  // Upload data if WiFi is available
  if (wifiConnected) {
    bool uploadSuccess = uploadSensorData(readings);
    if (uploadSuccess) {
      Serial.println("✓ All data uploaded successfully!");
    } else {
      Serial.println("⚠ Some data uploads failed");
    }
    
    // Wait for final uploads to complete
    delay(2000);
  } else {
    Serial.println("⚠ No WiFi connection - data not uploaded");
  }
  
  // Print summary
  Serial.println("=== Session Summary ===");
  Serial.printf("DHT11 (%s): %s\n", DHT_LOCATION, 
               readings.dht_success ? "SUCCESS" : "FAILED");
  Serial.printf("DS18B20 (%s): %s\n", DS18B20_LOCATION, 
               readings.ds18b20_success ? "SUCCESS" : "FAILED");
  Serial.printf("WiFi: %s\n", wifiConnected ? "CONNECTED" : "FAILED");
  Serial.println("=====================");
  
  // Enter deep sleep
  enterDeepSleep();
}

void loop() {
  // This will never be reached because we go to deep sleep in setup()
  // Deep sleep resets the ESP32, so it starts from setup() again
}