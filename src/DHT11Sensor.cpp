#include "DHT11Sensor.h"

DHT11Sensor::DHT11Sensor(const String& location) 
    : location(location), dht(Config::DHT_PIN, DHT11), lastReadTime(0) {
}

bool DHT11Sensor::initialize() {
    Serial.println("Initializing DHT11 sensor...");
    
    try {
        dht.begin();
        initialized = true;
        lastError = "";
        
        Serial.printf("✓ DHT11 sensor initialized at location: %s\n", location.c_str());
        return true;
    } catch (const std::exception& e) {
        setError("DHT11 initialization failed: " + String(e.what()));
        initialized = false;
        return false;
    }
}

bool DHT11Sensor::isReady() const {
    if (!initialized) {
        return false;
    }
    
    // DHT11 requires minimum interval between readings
    unsigned long currentTime = millis();
    return (currentTime - lastReadTime) >= MIN_READ_INTERVAL_MS;
}

bool DHT11Sensor::readSensor(std::vector<Reading>& readings) {
    readings.clear();
    
    if (!initialized) {
        setError("DHT11 not initialized");
        return false;
    }
    
    if (!isReady()) {
        setError("DHT11 not ready for reading (minimum interval not met)");
        return false;
    }
    
    Serial.println("Reading DHT11 sensor...");
    
    // Allow stabilization time
    delay(Config::DHT_STABILIZATION_DELAY_MS);
    
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    lastReadTime = millis();
    
    // Validate readings
    bool tempValid = isValidReading(temperature);
    bool humidValid = isValidReading(humidity);
    
    if (!tempValid && !humidValid) {
        setError("DHT11 failed to read both temperature and humidity");
        return false;
    }
    
    // Add temperature reading
    if (tempValid) {
        Reading tempReading(temperature, Status::SUCCESS);
        readings.push_back(tempReading);
        Serial.printf("✓ DHT11 Temperature: %.1f°C\n", temperature);
    } else {
        Reading tempReading;
        tempReading.status = Status::INVALID_DATA;
        tempReading.errorMessage = "Invalid temperature reading";
        readings.push_back(tempReading);
        Serial.println("✗ DHT11 Temperature: Invalid reading");
    }
    
    // Add humidity reading
    if (humidValid) {
        Reading humidReading(humidity, Status::SUCCESS);
        readings.push_back(humidReading);
        Serial.printf("✓ DHT11 Humidity: %.1f%%\n", humidity);
    } else {
        Reading humidReading;
        humidReading.status = Status::INVALID_DATA;
        humidReading.errorMessage = "Invalid humidity reading";
        readings.push_back(humidReading);
        Serial.println("✗ DHT11 Humidity: Invalid reading");
    }
    
    return tempValid || humidValid; // Success if at least one reading is valid
}

bool DHT11Sensor::isValidReading(float value) const {
    return !isnan(value) && isfinite(value);
}