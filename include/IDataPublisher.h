#pragma once

#include <Arduino.h>
#include <vector>
#include "ISensor.h"

/**
 * @brief Abstract interface for data publishers
 * 
 * Allows different implementations for data publishing (Supabase, MQTT, etc.)
 * Provides a common interface for sending sensor data to various destinations.
 */
class IDataPublisher {
public:
    struct PublishResult {
        bool success;
        int responseCode;
        String errorMessage;
        unsigned long timestamp;

        PublishResult() : success(false), responseCode(0), timestamp(0) {}
        PublishResult(bool success, int code = 200) 
            : success(success), responseCode(code), timestamp(millis()) {}
    };

    virtual ~IDataPublisher() = default;

    /**
     * @brief Initialize the publisher
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Check if publisher is ready to send data
     * @return true if ready
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Publish a single sensor reading
     * @param location Sensor location identifier
     * @param type Data type (temperature, humidity, co2, etc.)
     * @param value Sensor reading value
     * @return PublishResult containing success status and details
     */
    virtual PublishResult publish(const String& location, const String& type, float value) = 0;

    /**
     * @brief Publish multiple readings from a sensor
     * @param sensorName Name/type of the sensor
     * @param location Sensor location
     * @param readings Vector of sensor readings
     * @param dataTypes Vector of data type names (temperature, humidity, etc.)
     * @return Number of successfully published readings
     */
    virtual int publishBatch(const String& sensorName, const String& location, 
                           const std::vector<ISensor::Reading>& readings,
                           const std::vector<String>& dataTypes) = 0;

    /**
     * @brief Get publisher name/type
     */
    virtual String getName() const = 0;

    /**
     * @brief Get last error message
     */
    virtual String getLastError() const { return lastError; }

protected:
    String lastError;

    void setError(const String& error) {
        lastError = error;
        Serial.println("Publisher Error: " + error);
    }
};