#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"

/**
 * @brief WiFi connection manager
 * 
 * Handles WiFi initialization, connection, and status monitoring.
 * Provides retry logic and connection state management.
 */
class WiFiManager {
public:
    enum class Status {
        CONNECTED,
        DISCONNECTED,
        CONNECTING,
        FAILED,
        TIMEOUT
    };

    /**
     * @brief Constructor
     */
    WiFiManager() = default;

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~WiFiManager();

    /**
     * @brief Initialize and connect to WiFi
     * @param ssid Network SSID
     * @param password Network password
     * @param timeoutMs Connection timeout in milliseconds
     * @return true if connected successfully
     */
    bool connect(const char* ssid, const char* password, uint32_t timeoutMs = Config::WIFI_TIMEOUT_MS);

    /**
     * @brief Disconnect from WiFi and cleanup
     */
    void disconnect();

    /**
     * @brief Check if WiFi is connected
     */
    bool isConnected() const;

    /**
     * @brief Get current WiFi status
     */
    Status getStatus() const;

    /**
     * @brief Get local IP address
     */
    String getLocalIP() const;

    /**
     * @brief Get signal strength (RSSI)
     */
    int32_t getSignalStrength() const;

    /**
     * @brief Get last error message
     */
    String getLastError() const { return lastError; }

    /**
     * @brief Print connection information
     */
    void printConnectionInfo() const;

private:
    String lastError;
    unsigned long connectionStartTime;

    void setError(const String& error);
};