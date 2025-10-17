#pragma once

#include "IDataPublisher.h"
#include "Config.h"
#include <ESPSupabase.h>

/**
 * @brief Supabase data publisher implementation
 * 
 * Implements the IDataPublisher interface for sending data to Supabase database.
 * Handles authentication, JSON formatting, and HTTP communication.
 */
class SupabasePublisher : public IDataPublisher {
public:
    /**
     * @brief Constructor
     * @param url Supabase project URL
     * @param apiKey Supabase API key
     * @param tableName Database table name
     */
    SupabasePublisher(const String& url, const String& apiKey, const String& tableName = Config::SUPABASE_TABLE_NAME);

    /**
     * @brief Destructor
     */
    ~SupabasePublisher() override = default;

    // IDataPublisher interface implementation
    bool initialize() override;
    bool isReady() const override;
    PublishResult publish(const String& location, const String& type, float value) override;
    int publishBatch(const String& sensorName, const String& location, 
                    const std::vector<ISensor::Reading>& readings,
                    const std::vector<String>& dataTypes) override;
    String getName() const override { return "Supabase"; }

    /**
     * @brief Set database table name
     */
    void setTableName(const String& tableName) { this->tableName = tableName; }

    /**
     * @brief Get database table name
     */
    String getTableName() const { return tableName; }

private:
    String url;
    String apiKey;
    String tableName;
    Supabase supabase;
    bool initialized;

    /**
     * @brief Create JSON payload for sensor data
     */
    String createPayload(const String& location, const String& type, float value) const;

    /**
     * @brief Check if HTTP response indicates success
     */
    bool isSuccessResponse(int responseCode) const;
};