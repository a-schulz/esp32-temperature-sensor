#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <ESPSupabase.h>
#include <SensirionI2cScd4x.h>
#include "credentials.h"

// ========== SENSOR CONFIGURATION ==========
// DHT11 Sensor Configuration
#define DHTPIN 4
#define DHTTYPE DHT11

// DS18B20 Sensor Configuration
#define ONE_WIRE_BUS 8

// SCD-41 Sensor Configuration (I2C)
#define SDA_PIN 10  // I2C SDA pin for ESP32-C3
#define SCL_PIN 9   // I2C SCL pin for ESP32-C3

// Location Configuration - Change these for different deployments
const char* DHT_LOCATION = "alex-room";      // Location for DHT11 sensor
const char* DS18B20_LOCATION = "alex-outside"; // Location for DS18B20 sensor
const char* SCD41_LOCATION = "alex-room"; // Location for SCD-41 sensor

// ========== DEEP SLEEP CONFIGURATION ==========
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 900          // Time ESP32 will go to sleep (in seconds) - 15 minutes

// ========== SENSOR OBJECTS ==========
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
Supabase supabase;
SensirionI2cScd4x scd4x;

// ========== RTC MEMORY (survives deep sleep) ==========
RTC_DATA_ATTR int bootCount = 0;

// ========== SCD-41 ERROR HANDLING ==========
// Define NO_ERROR macro for SCD-41
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

static char errorMessage[64];

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
  float scd41_co2 = NAN;
  bool dht_success = false;
  bool ds18b20_success = false;
  bool scd41_success = false;
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

  // Read SCD-41 sensor with extended initialization time
  Serial.println("Reading SCD-41...");
  
  // Wait additional time for sensor to stabilize after startup
  delay(6000); // SCD-41 needs ~5 seconds for first measurement after startup
  
  // Check if data is ready with more conservative timeout
  bool dataReady = false;
  unsigned long startTime = millis();
  int16_t error;
  int attempts = 0;
  const int maxAttempts = 100; // 10 seconds max wait time
  
  do {
    error = scd4x.getDataReadyStatus(dataReady);
    attempts++;
    
    if (error != NO_ERROR) {
      // If we get communication error, try to re-establish connection
      if (attempts < 5) {
        Serial.printf("SCD-41 communication retry %d/5...\n", attempts);
        delay(500);
        continue;
      } else {
        Serial.print("SCD-41 getDataReadyStatus error after retries: ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        readings.scd41_success = false;
        return readings;
      }
    }
    
    if (!dataReady) {
      delay(100);
    } else {
      Serial.printf("✓ SCD-41 data ready after %d attempts (%.1fs)\n", 
                    attempts, (millis() - startTime) / 1000.0);
    }
  } while (!dataReady && attempts < maxAttempts);
  
  if (!dataReady) {
    Serial.printf("✗ SCD-41: Data not ready after %d attempts (timeout)\n", attempts);
    readings.scd41_success = false;
  } else {
    // Data is ready, read the measurement
    uint16_t co2;
    float temperature_scd41;
    float humidity_scd41;
    
    error = scd4x.readMeasurement(co2, temperature_scd41, humidity_scd41);
    
    if (error != NO_ERROR) {
      Serial.print("SCD-41 readMeasurement error: ");
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
      readings.scd41_success = false;
    } else if (co2 == 0) {
      Serial.println("✗ SCD-41: Invalid CO2 reading (0 ppm) - sensor may need more time");
      readings.scd41_success = false;
    } else {
      readings.scd41_co2 = co2;
      Serial.printf("✓ SCD-41: %d ppm CO2 (%.1f°C, %.1f%% RH)\n", 
                    co2, temperature_scd41, humidity_scd41);
      readings.scd41_success = true;
    }
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

  // Upload SCD-41 data
  if (readings.scd41_success) {
    if (!sendToSupabase(SCD41_LOCATION, "co2", readings.scd41_co2)) {
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
  Serial.printf("SCD-41 Location: %s\n", SCD41_LOCATION);
  Serial.printf("Sleep Duration: %d seconds\n", TIME_TO_SLEEP);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  print_wakeup_reason();
  Serial.println("========================");
}

// ========== I2C SCANNER FUNCTION ==========
void scanI2CDevices() {
  Serial.println("=== I2C Device Scanner ===");
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C addresses...");

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.printf("Found %d I2C device(s)\n", nDevices);
  }
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

  // Initialize I2C with custom pins and slower speed
  Serial.println("Initializing I2C...");
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(50000); // Use slower I2C clock (50kHz) for better reliability
  delay(500); // Extended delay for I2C stabilization
  
  // Scan for I2C devices
  scanI2CDevices();

  // Initialize SCD-41 with simplified approach
  Serial.println("Initializing SCD-41...");
  
  // Power cycle simulation with delays
  delay(1000); // Wait for sensor to be ready
  
  scd4x.begin(Wire, 0x62);
  delay(200); // Give sensor time to respond
  
  // Try a simple wake-up first
  int16_t error = scd4x.wakeUp();
  if (error == NO_ERROR) {
    Serial.println("✓ SCD-41 wake-up successful");
    delay(20);
    
    // Try to stop any ongoing measurements
    error = scd4x.stopPeriodicMeasurement();
    if (error == NO_ERROR) {
      Serial.println("✓ SCD-41 stopped periodic measurement");
    } else {
      Serial.println("⚠ SCD-41 stop measurement failed (might not be running)");
    }
    
    // Wait before starting measurements
    delay(1000);
    
    // Start periodic measurements without reinit
    error = scd4x.startPeriodicMeasurement();
    if (error == NO_ERROR) {
      Serial.println("✓ SCD-41 periodic measurement started successfully");
      Serial.println("  Note: First valid measurement available after ~5 seconds");
    } else {
      Serial.print("✗ SCD-41 start measurement failed: ");
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
    }
  } else {
    Serial.println("✗ SCD-41 communication failed");
    Serial.println("  Check wiring: VCC->3.3V, GND->GND, SDA->GPIO10, SCL->GPIO9");
  }
  
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
  Serial.printf("SCD-41 (%s): %s\n", SCD41_LOCATION, 
               readings.scd41_success ? "SUCCESS" : "FAILED");
  Serial.printf("WiFi: %s\n", wifiConnected ? "CONNECTED" : "FAILED");
  Serial.println("=====================");
  
  // Enter deep sleep
  enterDeepSleep();
}

void loop() {
  // This will never be reached because we go to deep sleep in setup()
  // Deep sleep resets the ESP32, so it starts from setup() again
}