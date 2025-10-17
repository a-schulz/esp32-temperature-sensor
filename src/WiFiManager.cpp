#include "WiFiManager.h"

WiFiManager::~WiFiManager() {
    disconnect();
}

bool WiFiManager::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    Serial.println("=== WiFi Connection ===");
    Serial.printf("Connecting to: %s\n", ssid);
    
    connectionStartTime = millis();
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED && (millis() - connectionStartTime) < timeoutMs) {
        delay(Config::WIFI_RETRY_DELAY_MS);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi connected successfully!");
        printConnectionInfo();
        lastError = "";
        return true;
    } else {
        setError("WiFi connection timeout after " + String(timeoutMs) + "ms");
        return false;
    }
}

void WiFiManager::disconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Disconnecting WiFi...");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

WiFiManager::Status WiFiManager::getStatus() const {
    switch (WiFi.status()) {
        case WL_CONNECTED:
            return Status::CONNECTED;
        case WL_IDLE_STATUS:
        case WL_SCAN_COMPLETED:
            return Status::CONNECTING;
        case WL_NO_SSID_AVAIL:
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
            return Status::FAILED;
        case WL_DISCONNECTED:
            return Status::DISCONNECTED;
        default:
            return Status::FAILED;
    }
}

String WiFiManager::getLocalIP() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int32_t WiFiManager::getSignalStrength() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

void WiFiManager::printConnectionInfo() const {
    if (isConnected()) {
        Serial.printf("IP Address: %s\n", getLocalIP().c_str());
        Serial.printf("Signal Strength: %d dBm\n", getSignalStrength());
        Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
    }
}

void WiFiManager::setError(const String& error) {
    lastError = error;
    Serial.println("WiFi Error: " + error);
}