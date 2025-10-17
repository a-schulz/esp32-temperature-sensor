#pragma once

#include <Arduino.h>
#include <vector>

/**
 * @brief Abstract base class for all environmental sensors
 * 
 * Provides a common interface for reading sensor data and checking sensor status.
 * All concrete sensor implementations should inherit from this class.
 */
class ISensor {
public:
    enum class Status {
        SUCCESS,
        FAILED,
        NOT_INITIALIZED,
        COMMUNICATION_ERROR,
        INVALID_DATA
    };

    struct Reading {
        float value;
        Status status;
        String errorMessage;
        unsigned long timestamp;

        Reading() : value(NAN), status(Status::NOT_INITIALIZED), timestamp(0) {}
        Reading(float val, Status stat = Status::SUCCESS) 
            : value(val), status(stat), timestamp(millis()) {}
    };

    virtual ~ISensor() = default;

    /**
     * @brief Initialize the sensor
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * @brief Check if sensor is ready for reading
     * @return true if ready, false otherwise
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Get sensor name/type
     * @return String containing sensor name
     */
    virtual String getName() const = 0;

    /**
     * @brief Get sensor location identifier
     * @return String containing location
     */
    virtual String getLocation() const = 0;

    /**
     * @brief Read sensor data
     * @param readings Vector to store readings (sensor may provide multiple values)
     * @return true if read successful, false otherwise
     */
    virtual bool readSensor(std::vector<Reading>& readings) = 0;

    /**
     * @brief Get last error message
     * @return String containing error description
     */
    virtual String getLastError() const { return lastError; }

protected:
    String lastError;
    bool initialized = false;

    void setError(const String& error) {
        lastError = error;
        Serial.println("Sensor Error: " + error);
    }
};