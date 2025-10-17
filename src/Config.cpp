#include "Config.h"

// Static member definitions
String Config::DHT_LOCATION;
String Config::DS18B20_LOCATION;
String Config::SCD41_LOCATION;
String Config::SUPABASE_TABLE_NAME;

void Config::initialize() {
    DHT_LOCATION = "alex-room";
    DS18B20_LOCATION = "alex-outside";
    SCD41_LOCATION = "alex-room";
    SUPABASE_TABLE_NAME = "environment_measurements";
}