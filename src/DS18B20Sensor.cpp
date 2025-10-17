#include "DS18B20Sensor.h"

DS18B20Sensor::DS18B20Sensor(const String& location, uint8_t deviceIndex) 
    : location(location), deviceIndex(deviceIndex), oneWire(Config::DS18B20_PIN), 
      dallas(&oneWire), lastConversionTime(0) {
}

bool DS18B20Sensor::initialize() {
    Serial.println("Initializing DS18B20 sensor...");
    
    try {
        dallas.begin();
        
        uint8_t deviceCount = dallas.getDeviceCount();
        Serial.printf("DS18B20 devices found: %d\n", deviceCount);
        
        if (deviceCount == 0) {
            setError("No DS18B20 devices found. Check wiring and pullup resistor.");
            initialized = false;
            return false;
        }
        
        if (deviceIndex >= deviceCount) {
            setError("Device index " + String(deviceIndex) + " exceeds available devices (" + String(deviceCount) + ")");
            initialized = false;
            return false;
        }
        
        Serial.printf("DS18B20 parasite power: %s\n", 
                     dallas.isParasitePowerMode() ? "ON" : "OFF");
        
        initialized = true;
        lastError = "";
        
        Serial.printf("✓ DS18B20 sensor initialized at location: %s (device %d)\n", 
                     location.c_str(), deviceIndex);
        return true;
    } catch (const std::exception& e) {
        setError("DS18B20 initialization failed: " + String(e.what()));
        initialized = false;
        return false;
    }
}

bool DS18B20Sensor::isReady() const {
    if (!initialized) {
        return false;
    }
    
    // Check if enough time has passed since last conversion
    unsigned long currentTime = millis();
    return (currentTime - lastConversionTime) >= Config::DS18B20_CONVERSION_DELAY_MS;
}

bool DS18B20Sensor::readSensor(std::vector<Reading>& readings) {
    readings.clear();
    
    if (!initialized) {
        setError("DS18B20 not initialized");
        return false;
    }
    
    Serial.println("Reading DS18B20 sensor...");
    
    // Request temperature conversion
    dallas.requestTemperatures();
    lastConversionTime = millis();
    
    // Wait for conversion to complete
    delay(Config::DS18B20_CONVERSION_DELAY_MS);
    
    float temperature = dallas.getTempCByIndex(deviceIndex);
    
    if (!isValidTemperature(temperature)) {
        setError("DS18B20 returned invalid temperature: " + String(temperature));
        
        Reading failedReading;
        failedReading.status = Status::INVALID_DATA;
        failedReading.errorMessage = lastError;
        readings.push_back(failedReading);
        return false;
    }
    
    Reading tempReading(temperature, Status::SUCCESS);
    readings.push_back(tempReading);
    
    Serial.printf("✓ DS18B20 Temperature: %.1f°C\n", temperature);
    return true;
}

uint8_t DS18B20Sensor::getDeviceCount() {
    if (!initialized) {
        return 0;
    }
    return dallas.getDeviceCount();
}

bool DS18B20Sensor::isParasitePowerMode() {
    if (!initialized) {
        return false;
    }
    return dallas.isParasitePowerMode();
}

bool DS18B20Sensor::isValidTemperature(float temperature) const {
    return temperature != DEVICE_DISCONNECTED_C && 
           temperature != INVALID_TEMPERATURE &&
           !isnan(temperature) && 
           isfinite(temperature) &&
           temperature > -55.0f && temperature < 125.0f; // DS18B20 valid range
}