#pragma once

#include <Arduino.h>

/**
 * @brief Configuration manager for sensor system
 * 
 * Centralized configuration management for all system components.
 * Provides compile-time and runtime configuration options.
 */
class Config {
public:
    // Deep Sleep Configuration
    static constexpr unsigned long SLEEP_DURATION_SECONDS = 900; // 15 minutes
    static constexpr uint64_t uS_TO_S_FACTOR = 1000000ULL;

    // I2C Configuration for ESP32-C3
    static constexpr uint8_t I2C_SDA_PIN = 10;
    static constexpr uint8_t I2C_SCL_PIN = 9;
    static constexpr uint32_t I2C_FREQUENCY = 50000; // 50kHz for reliability

    // DHT11 Configuration
    static constexpr uint8_t DHT_PIN = 4;
    static constexpr uint16_t DHT_STABILIZATION_DELAY_MS = 2000;

    // DS18B20 Configuration  
    static constexpr uint8_t DS18B20_PIN = 8;
    static constexpr uint16_t DS18B20_CONVERSION_DELAY_MS = 1000;

    // SCD-41 Configuration
    static constexpr uint8_t SCD41_I2C_ADDRESS = 0x62;
    static constexpr uint16_t SCD41_STARTUP_DELAY_MS = 6000;
    static constexpr uint16_t SCD41_RETRY_ATTEMPTS = 100;
    static constexpr uint16_t SCD41_RETRY_DELAY_MS = 100;

    // WiFi Configuration
    static constexpr uint32_t WIFI_TIMEOUT_MS = 30000;
    static constexpr uint16_t WIFI_RETRY_DELAY_MS = 500;

    // Serial Configuration
    static constexpr uint32_t SERIAL_BAUD_RATE = 115200;

    // Sensor Locations (can be overridden at runtime)
    static String DHT_LOCATION;
    static String DS18B20_LOCATION;
    static String SCD41_LOCATION;

    // Supabase Configuration
    static String SUPABASE_TABLE_NAME;

    /**
     * @brief Initialize default configuration values
     */
    static void initialize();

    /**
     * @brief Set sensor location at runtime
     */
    static void setDHTLocation(const String& location) { DHT_LOCATION = location; }
    static void setDS18B20Location(const String& location) { DS18B20_LOCATION = location; }
    static void setSCD41Location(const String& location) { SCD41_LOCATION = location; }
    static void setSupabaseTable(const String& table) { SUPABASE_TABLE_NAME = table; }
};