#include "SCD41Sensor.h"

char SCD41Sensor::errorMessage[64];

SCD41Sensor::SCD41Sensor(const String& location, uint8_t i2cAddress)
    : location(location), i2cAddress(i2cAddress), initializationTime(0), measurementStarted(false) {
}

bool SCD41Sensor::initialize() {
    Serial.println("Initializing SCD-41 sensor...");
    
    if (!initializeI2C()) {
        return false;
    }
    
    // Scan for I2C devices
    scanI2CDevices();
    
    // Initialize sensor communication
    scd4x.begin(Wire, i2cAddress);
    delay(200); // Allow sensor to respond
    
    // Try wake-up
    int16_t error = scd4x.wakeUp();
    if (error == NO_ERROR) {
        Serial.printf("✓ SCD-41 responds at address 0x%02X\n", i2cAddress);
    } else {
        setError("SCD-41 wake-up failed: " + getErrorString(error));
        return false;
    }
    
    delay(20);
    
    if (!startMeasurement()) {
        return false;
    }
    
    initializationTime = millis();
    initialized = true;
    lastError = "";
    
    Serial.printf("✓ SCD-41 sensor initialized at location: %s\n", location.c_str());
    Serial.println("  Note: First valid measurement available after ~5 seconds");
    
    return true;
}

bool SCD41Sensor::initializeI2C() {
    Serial.println("Initializing I2C bus...");
    Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
    Wire.setClock(Config::I2C_FREQUENCY);
    delay(500); // Allow I2C to stabilize
    
    return true;
}

bool SCD41Sensor::startMeasurement() {
    // Stop any ongoing measurements first
    int16_t error = scd4x.stopPeriodicMeasurement();
    if (error == NO_ERROR) {
        Serial.println("✓ SCD-41 stopped any ongoing measurements");
    } else {
        Serial.println("⚠ SCD-41 stop measurement (sensor might not be running)");
    }
    
    delay(1000); // Wait before starting new measurements
    
    // Start periodic measurements
    error = scd4x.startPeriodicMeasurement();
    if (error != NO_ERROR) {
        setError("SCD-41 start measurement failed: " + getErrorString(error));
        measurementStarted = false;
        return false;
    }
    
    Serial.println("✓ SCD-41 periodic measurement started");
    measurementStarted = true;
    return true;
}

bool SCD41Sensor::isReady() const {
    if (!initialized || !measurementStarted) {
        return false;
    }
    
    // SCD-41 needs time after initialization for valid readings
    unsigned long currentTime = millis();
    return (currentTime - initializationTime) >= Config::SCD41_STARTUP_DELAY_MS;
}

bool SCD41Sensor::readSensor(std::vector<Reading>& readings) {
    readings.clear();
    
    if (!initialized) {
        setError("SCD-41 not initialized");
        return false;
    }
    
    if (!isReady()) {
        setError("SCD-41 not ready (waiting for startup delay)");
        return false;
    }
    
    Serial.println("Reading SCD-41 sensor...");
    
    if (!waitForDataReady()) {
        return false;
    }
    
    // Read measurement
    uint16_t co2;
    float temperature;
    float humidity;
    
    int16_t error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error != NO_ERROR) {
        setError("SCD-41 read measurement failed: " + getErrorString(error));
        return false;
    }
    
    // Validate and store readings
    bool hasValidReading = false;
    
    // CO2 reading
    if (isValidCO2(co2)) {
        Reading co2Reading(co2, Status::SUCCESS);
        readings.push_back(co2Reading);
        Serial.printf("✓ SCD-41 CO2: %d ppm\n", co2);
        hasValidReading = true;
    } else {
        Reading co2Reading;
        co2Reading.status = Status::INVALID_DATA;
        co2Reading.errorMessage = "Invalid CO2 reading: " + String(co2);
        readings.push_back(co2Reading);
        Serial.printf("✗ SCD-41 CO2: Invalid (%d ppm)\n", co2);
    }
    
    // Temperature reading (optional - for reference only)
    if (isValidTemperature(temperature)) {
        Reading tempReading(temperature, Status::SUCCESS);
        readings.push_back(tempReading);
        Serial.printf("  SCD-41 Temperature: %.1f°C\n", temperature);
    }
    
    // Humidity reading (optional - for reference only)
    if (isValidHumidity(humidity)) {
        Reading humidReading(humidity, Status::SUCCESS);
        readings.push_back(humidReading);
        Serial.printf("  SCD-41 Humidity: %.1f%%\n", humidity);
    }
    
    return hasValidReading;
}

bool SCD41Sensor::waitForDataReady() {
    bool dataReady = false;
    int attempts = 0;
    
    do {
        int16_t error = scd4x.getDataReadyStatus(dataReady);
        attempts++;
        
        if (error != NO_ERROR) {
            if (attempts < 5) {
                Serial.printf("SCD-41 communication retry %d/5...\n", attempts);
                delay(500);
                continue;
            } else {
                setError("SCD-41 data ready check failed after retries: " + getErrorString(error));
                return false;
            }
        }
        
        if (!dataReady) {
            delay(Config::SCD41_RETRY_DELAY_MS);
        }
    } while (!dataReady && attempts < Config::SCD41_RETRY_ATTEMPTS);
    
    if (!dataReady) {
        setError("SCD-41 data not ready after " + String(attempts) + " attempts");
        return false;
    }
    
    Serial.printf("✓ SCD-41 data ready after %d attempts\n", attempts);
    return true;
}

void SCD41Sensor::scanI2CDevices() {
    Serial.println("=== I2C Device Scanner ===");
    byte error, address;
    int nDevices = 0;
    
    Serial.println("Scanning I2C addresses...");
    
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            nDevices++;
        }
    }
    
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.printf("Found %d I2C device(s)\n", nDevices);
    }
    Serial.println("========================");
}

bool SCD41Sensor::isValidCO2(uint16_t co2) const {
    return co2 >= MIN_VALID_CO2 && co2 <= MAX_VALID_CO2;
}

bool SCD41Sensor::isValidTemperature(float temperature) const {
    return !isnan(temperature) && isfinite(temperature) &&
           temperature >= MIN_VALID_TEMP && temperature <= MAX_VALID_TEMP;
}

bool SCD41Sensor::isValidHumidity(float humidity) const {
    return !isnan(humidity) && isfinite(humidity) &&
           humidity >= MIN_VALID_HUMIDITY && humidity <= MAX_VALID_HUMIDITY;
}

String SCD41Sensor::getErrorString(int16_t error) const {
    errorToString(error, errorMessage, sizeof(errorMessage));
    return String(errorMessage);
}