#pragma once

#include "ISensor.h"
#include "Config.h"
#include <DHT.h>

/**
 * @brief DHT11 Temperature and Humidity Sensor
 * 
 * Implements the ISensor interface for DHT11 sensor.
 * Provides temperature and humidity readings.
 */
class DHT11Sensor : public ISensor {
public:
    enum class ReadingType {
        TEMPERATURE = 0,
        HUMIDITY = 1
    };

    /**
     * @brief Constructor
     * @param location Sensor location identifier
     */
    explicit DHT11Sensor(const String& location = Config::DHT_LOCATION);

    /**
     * @brief Destructor
     */
    ~DHT11Sensor() override = default;

    // ISensor interface implementation
    bool initialize() override;
    bool isReady() const override;
    String getName() const override { return "DHT11"; }
    String getLocation() const override { return location; }
    bool readSensor(std::vector<Reading>& readings) override;

private:
    String location;
    DHT dht;
    unsigned long lastReadTime;
    static constexpr unsigned long MIN_READ_INTERVAL_MS = 2000;

    bool isValidReading(float value) const;
};