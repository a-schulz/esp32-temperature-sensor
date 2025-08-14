-- Simple sensor database schema
CREATE TABLE IF NOT EXISTS temperature_readings (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    device_id VARCHAR(50) NOT NULL,
    location VARCHAR(50) NOT NULL,
    temperature FLOAT NOT NULL,
    rssi INTEGER,
    battery_level FLOAT,
    metadata JSONB
);

-- Index for fast time-based queries
CREATE INDEX IF NOT EXISTS idx_temperature_timestamp ON temperature_readings(timestamp);
CREATE INDEX IF NOT EXISTS idx_temperature_device ON temperature_readings(device_id);
CREATE INDEX IF NOT EXISTS idx_temperature_location ON temperature_readings(location);

-- Insert some test data
INSERT INTO temperature_readings (device_id, location, temperature, rssi)
VALUES ('heating_sensor_01', 'boiler_room', 22.5, -45);

-- Create a simple view for Grafana
CREATE OR REPLACE VIEW latest_temperatures AS
SELECT DISTINCT ON (location)
    location,
    temperature,
    timestamp,
    device_id,
    rssi
FROM temperature_readings
ORDER BY location, timestamp DESC;
