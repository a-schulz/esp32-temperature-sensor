/**
 * @file modular_sensor_system.cpp
 * @brief Professional modular sensor system implementation
 * 
 * This file demonstrates the new modular architecture with proper separation of concerns.
 * All components are reusable and can be easily extended or replaced.
 */

#include <Arduino.h>
#include <esp_sleep.h>

// Configuration and interfaces
#include "Config.h"
#include "credentials.h"

// Sensor implementations
#include "DHT11Sensor.h"
#include "DS18B20Sensor.h"
#include "SCD41Sensor.h"

// Network and data publishing
#include "WiFiManager.h"
#include "SupabasePublisher.h"

// ========== GLOBAL SYSTEM COMPONENTS ==========
WiFiManager wifiManager;
SupabasePublisher dataPublisher(SUPABASE_URL, SUPABASE_KEY);

// Sensor instances
DHT11Sensor dht11Sensor(Config::DHT_LOCATION);
DS18B20Sensor ds18b20Sensor(Config::DS18B20_LOCATION);
SCD41Sensor scd41Sensor(Config::SCD41_LOCATION);

// System state
RTC_DATA_ATTR int bootCount = 0;

// ========== SYSTEM FUNCTIONS ==========

void printSystemInfo() {
    Serial.println("========================================");
    Serial.println("      Professional Sensor System       ");
    Serial.println("========================================");
    Serial.printf("Boot Count: %d\n", bootCount);
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Sleep Duration: %d seconds\n", Config::SLEEP_DURATION_SECONDS);
    
    // Print wakeup reason
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup: Timer (scheduled measurement)");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Wakeup: External signal");
            break;
        default:
            Serial.println("Wakeup: Power-on or reset");
            break;
    }
    Serial.println("========================================");
}

bool initializeSystem() {
    Serial.println("\n=== System Initialization ===");
    
    // Initialize configuration
    Config::initialize();
    
    bool allSuccess = true;
    
    // Initialize all sensors
    Serial.println("Initializing sensors...");
    
    if (!dht11Sensor.initialize()) {
        Serial.printf("⚠ %s initialization failed: %s\n", 
                     dht11Sensor.getName().c_str(), dht11Sensor.getLastError().c_str());
        allSuccess = false;
    }
    
    if (!ds18b20Sensor.initialize()) {
        Serial.printf("⚠ %s initialization failed: %s\n", 
                     ds18b20Sensor.getName().c_str(), ds18b20Sensor.getLastError().c_str());
        allSuccess = false;
    }
    
    if (!scd41Sensor.initialize()) {
        Serial.printf("⚠ %s initialization failed: %s\n", 
                     scd41Sensor.getName().c_str(), scd41Sensor.getLastError().c_str());
        allSuccess = false;
    }
    
    // Initialize network
    Serial.println("\nInitializing network...");
    bool wifiConnected = wifiManager.connect(WIFI_SSID, WIFI_PASSWORD);
    
    if (wifiConnected) {
        // Initialize data publisher
        if (!dataPublisher.initialize()) {
            Serial.printf("⚠ %s initialization failed: %s\n", 
                         dataPublisher.getName().c_str(), dataPublisher.getLastError().c_str());
            allSuccess = false;
        }
    } else {
        Serial.printf("⚠ WiFi connection failed: %s\n", wifiManager.getLastError().c_str());
        allSuccess = false;
    }
    
    Serial.printf("\n%s System initialization %s\n", 
                 allSuccess ? "✓" : "⚠", 
                 allSuccess ? "completed successfully" : "completed with warnings");
    
    return allSuccess;
}

void readAndPublishSensorData() {
    Serial.println("\n=== Sensor Data Collection ===");
    
    std::vector<ISensor::Reading> readings;
    int totalPublished = 0;
    bool hasPublisher = dataPublisher.isReady();
    
    // DHT11 Sensor (Temperature + Humidity)
    Serial.println("\nProcessing DHT11 sensor...");
    if (dht11Sensor.isReady() && dht11Sensor.readSensor(readings)) {
        if (hasPublisher) {
            std::vector<String> dhtDataTypes = {"temperature", "humidity"};
            int published = dataPublisher.publishBatch(
                dht11Sensor.getName(), 
                dht11Sensor.getLocation(), 
                readings, 
                dhtDataTypes
            );
            totalPublished += published;
        }
    } else {
        Serial.printf("⚠ %s read failed: %s\n", 
                     dht11Sensor.getName().c_str(), dht11Sensor.getLastError().c_str());
    }
    
    // DS18B20 Sensor (Temperature)
    Serial.println("\nProcessing DS18B20 sensor...");
    if (ds18b20Sensor.isReady() && ds18b20Sensor.readSensor(readings)) {
        if (hasPublisher) {
            std::vector<String> ds18b20DataTypes = {"temperature"};
            int published = dataPublisher.publishBatch(
                ds18b20Sensor.getName(), 
                ds18b20Sensor.getLocation(), 
                readings, 
                ds18b20DataTypes
            );
            totalPublished += published;
        }
    } else {
        Serial.printf("⚠ %s read failed: %s\n", 
                     ds18b20Sensor.getName().c_str(), ds18b20Sensor.getLastError().c_str());
    }
    
    // SCD-41 Sensor (CO2 + Temperature + Humidity)
    Serial.println("\nProcessing SCD-41 sensor...");
    if (scd41Sensor.isReady() && scd41Sensor.readSensor(readings)) {
        if (hasPublisher && !readings.empty()) {
            // Only publish CO2 data (first reading) to avoid duplicate temperature/humidity
            if (readings[0].status == ISensor::Status::SUCCESS) {
                auto result = dataPublisher.publish(
                    scd41Sensor.getLocation(), 
                    "co2", 
                    readings[0].value
                );
                if (result.success) {
                    totalPublished++;
                }
            }
        }
    } else {
        Serial.printf("⚠ %s read failed: %s\n", 
                     scd41Sensor.getName().c_str(), scd41Sensor.getLastError().c_str());
    }
    
    // Summary
    Serial.println("\n=== Data Collection Summary ===");
    Serial.printf("Sensors processed: 3\n");
    Serial.printf("Data points published: %d\n", totalPublished);
    Serial.printf("Publisher: %s (%s)\n", 
                 dataPublisher.getName().c_str(),
                 hasPublisher ? "Connected" : "Offline");
    
    if (!hasPublisher) {
        Serial.println("⚠ Data not published - no network connection");
    }
}

void enterDeepSleep() {
    Serial.println("\n=== Preparing Deep Sleep ===");
    
    // Cleanup network resources
    wifiManager.disconnect();
    
    // Configure wake-up timer
    esp_sleep_enable_timer_wakeup(Config::SLEEP_DURATION_SECONDS * Config::uS_TO_S_FACTOR);
    
    Serial.printf("Configured for %d seconds sleep\n", Config::SLEEP_DURATION_SECONDS);
    Serial.println("Entering deep sleep...");
    Serial.flush();
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void setup() {
    // Initialize serial communication
    Serial.begin(Config::SERIAL_BAUD_RATE);
    delay(1000);
    
    // Increment boot counter
    ++bootCount;
    
    // Print system information
    printSystemInfo();
    
    // Initialize all system components
    bool systemReady = initializeSystem();
    
    // Read sensors and publish data
    readAndPublishSensorData();
    
    // Allow time for final operations
    delay(2000);
    
    // Enter deep sleep for power conservation
    enterDeepSleep();
}

void loop() {
    // This should never be reached due to deep sleep
    // If we get here, something went wrong with deep sleep
    Serial.println("ERROR: Loop reached - deep sleep failed!");
    delay(5000);
    ESP.restart();
}