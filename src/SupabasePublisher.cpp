#include "SupabasePublisher.h"
#include <WiFi.h>

SupabasePublisher::SupabasePublisher(const String& url, const String& apiKey, const String& tableName)
    : url(url), apiKey(apiKey), tableName(tableName), initialized(false) {
}

bool SupabasePublisher::initialize() {
    Serial.println("Initializing Supabase publisher...");
    
    if (url.isEmpty() || apiKey.isEmpty()) {
        setError("Supabase URL or API key is empty");
        return false;
    }
    
    if (!isReady()) {
        setError("WiFi not connected - cannot initialize Supabase");
        return false;
    }
    
    try {
        supabase.begin(url, apiKey);
        initialized = true;
        lastError = "";
        
        Serial.printf("✓ Supabase publisher initialized\n");
        Serial.printf("  URL: %s\n", url.c_str());
        Serial.printf("  Table: %s\n", tableName.c_str());
        return true;
    } catch (const std::exception& e) {
        setError("Supabase initialization failed: " + String(e.what()));
        initialized = false;
        return false;
    }
}

bool SupabasePublisher::isReady() const {
    return initialized && WiFi.status() == WL_CONNECTED;
}

IDataPublisher::PublishResult SupabasePublisher::publish(const String& location, const String& type, float value) {
    PublishResult result;
    
    if (!isReady()) {
        result.errorMessage = "Publisher not ready (WiFi disconnected or not initialized)";
        setError(result.errorMessage);
        return result;
    }
    
    String payload = createPayload(location, type, value);
    Serial.printf("Publishing to Supabase: %s\n", payload.c_str());
    
    int response = supabase.insert(tableName, payload, false);
    result.responseCode = response;
    
    if (isSuccessResponse(response)) {
        result.success = true;
        Serial.println("✓ Data published successfully!");
    } else {
        result.success = false;
        result.errorMessage = "HTTP error: " + String(response);
        setError("Failed to publish data: " + result.errorMessage);
    }
    
    return result;
}

int SupabasePublisher::publishBatch(const String& sensorName, const String& location, 
                                   const std::vector<ISensor::Reading>& readings,
                                   const std::vector<String>& dataTypes) {
    if (readings.size() != dataTypes.size()) {
        setError("Mismatch between readings count and data types count");
        return 0;
    }
    
    int successCount = 0;
    
    for (size_t i = 0; i < readings.size(); i++) {
        const auto& reading = readings[i];
        
        // Only publish successful readings
        if (reading.status == ISensor::Status::SUCCESS) {
            PublishResult result = publish(location, dataTypes[i], reading.value);
            if (result.success) {
                successCount++;
            }
            
            // Add delay between requests to avoid overwhelming the server
            delay(1000);
        } else {
            Serial.printf("⚠ Skipping invalid %s reading: %s\n", 
                         dataTypes[i].c_str(), reading.errorMessage.c_str());
        }
    }
    
    Serial.printf("Published %d/%d readings from %s sensor\n", 
                 successCount, (int)readings.size(), sensorName.c_str());
    
    return successCount;
}

String SupabasePublisher::createPayload(const String& location, const String& type, float value) const {
    return "{\"location\": \"" + location + 
           "\", \"type\": \"" + type + 
           "\", \"value\": " + String(value, 2) + "}";
}

bool SupabasePublisher::isSuccessResponse(int responseCode) const {
    return responseCode >= 200 && responseCode < 300;
}