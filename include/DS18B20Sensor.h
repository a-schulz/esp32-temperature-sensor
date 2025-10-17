#pragma once

#include "ISensor.h"
#include "Config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * @brief DS18B20 Digital Temperature Sensor
 * 
 * Implements the ISensor interface for DS18B20 OneWire temperature sensor.
 * Supports multiple sensors on the same bus.
 */
class DS18B20Sensor : public ISensor {
public:
    /**
     * @brief Constructor
     * @param location Sensor location identifier
     * @param deviceIndex Index of device on OneWire bus (default: 0)
     */
    explicit DS18B20Sensor(const String& location = Config::DS18B20_LOCATION, uint8_t deviceIndex = 0);

    /**
     * @brief Destructor
     */
    ~DS18B20Sensor() override = default;

    // ISensor interface implementation
    bool initialize() override;
    bool isReady() const override;
    String getName() const override { return "DS18B20"; }
    String getLocation() const override { return location; }
    bool readSensor(std::vector<Reading>& readings) override;

    /**
     * @brief Get number of devices found on the bus
     */
    uint8_t getDeviceCount();

    /**
     * @brief Check if sensor is using parasite power
     */
    bool isParasitePowerMode();

private:
    String location;
    uint8_t deviceIndex;
    OneWire oneWire;
    DallasTemperature dallas;
    unsigned long lastConversionTime;
    static constexpr float INVALID_TEMPERATURE = -127.0f;
    static constexpr unsigned long CONVERSION_TIMEOUT_MS = 2000;

    bool isValidTemperature(float temperature) const;
};