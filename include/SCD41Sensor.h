#pragma once

#include "ISensor.h"
#include "Config.h"
#include <SensirionI2cScd4x.h>
#include <Wire.h>

/**
 * @brief SCD-41 CO2, Temperature and Humidity Sensor
 * 
 * Implements the ISensor interface for SCD-41 I2C sensor.
 * Provides CO2, temperature, and humidity readings.
 */
class SCD41Sensor : public ISensor {
public:
    enum class ReadingType {
        CO2 = 0,
        TEMPERATURE = 1,
        HUMIDITY = 2
    };

    /**
     * @brief Constructor
     * @param location Sensor location identifier
     * @param i2cAddress I2C address (default: 0x62)
     */
    explicit SCD41Sensor(const String& location = Config::SCD41_LOCATION, 
                        uint8_t i2cAddress = Config::SCD41_I2C_ADDRESS);

    /**
     * @brief Destructor
     */
    ~SCD41Sensor() override = default;

    // ISensor interface implementation
    bool initialize() override;
    bool isReady() const override;
    String getName() const override { return "SCD-41"; }
    String getLocation() const override { return location; }
    bool readSensor(std::vector<Reading>& readings) override;

    /**
     * @brief Scan for I2C devices on the bus
     */
    static void scanI2CDevices();

private:
    String location;
    uint8_t i2cAddress;
    SensirionI2cScd4x scd4x;
    unsigned long initializationTime;
    bool measurementStarted;
    
    static char errorMessage[64];
    
    // SCD-41 specific constants
    static constexpr int16_t NO_ERROR = 0;
    static constexpr uint16_t MIN_VALID_CO2 = 1; // 0 ppm indicates sensor error
    static constexpr uint16_t MAX_VALID_CO2 = 40000; // Reasonable upper limit
    static constexpr float MIN_VALID_TEMP = -40.0f;
    static constexpr float MAX_VALID_TEMP = 70.0f;
    static constexpr float MIN_VALID_HUMIDITY = 0.0f;
    static constexpr float MAX_VALID_HUMIDITY = 100.0f;

    bool initializeI2C();
    bool startMeasurement();
    bool waitForDataReady();
    bool isValidCO2(uint16_t co2) const;
    bool isValidTemperature(float temperature) const;
    bool isValidHumidity(float humidity) const;
    String getErrorString(int16_t error) const;
};