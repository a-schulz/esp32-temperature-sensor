# Backend API Requirements & Data Architecture

## Overview
Define the backend data architecture and API requirements to support the temperature sensor dashboard, focusing on reliable data collection, storage, and retrieval from ESP32 sensors.

## Current Data Flow
```
ESP32 Sensors → Supabase → Frontend Dashboard
     ↓
MQTT Broker → PostgreSQL → Grafana (Alternative path)
```

## Supabase Schema Requirements

### Environment Measurements Table
```sql
CREATE TABLE environment_measurements (
    id uuid DEFAULT gen_random_uuid() PRIMARY KEY,
    created_at timestamp with time zone DEFAULT now(),
    location text NOT NULL,
    type text NOT NULL CHECK (type IN ('temperature', 'humidity')),
    value numeric NOT NULL,
    device_id text,
    rssi integer,
    battery_level numeric,
    metadata jsonb
);

-- Indexes for performance
CREATE INDEX idx_env_measurements_location_time ON environment_measurements(location, created_at DESC);
CREATE INDEX idx_env_measurements_type_time ON environment_measurements(type, created_at DESC);
CREATE INDEX idx_env_measurements_device ON environment_measurements(device_id);
```

### Row Level Security (RLS)
```sql
-- Enable RLS
ALTER TABLE environment_measurements ENABLE ROW LEVEL SECURITY;

-- Allow anonymous read access
CREATE POLICY "Allow anonymous read" ON environment_measurements
    FOR SELECT USING (true);

-- Allow authenticated insert (for sensors)
CREATE POLICY "Allow authenticated insert" ON environment_measurements
    FOR INSERT WITH CHECK (true);
```

## API Endpoints (Supabase REST)

### GET Current Readings
**Endpoint**: `/rest/v1/environment_measurements`
**Method**: GET
**Query Parameters**:
- `location=eq.garage` - Filter by location
- `type=eq.temperature` - Filter by measurement type
- `order=created_at.desc` - Order by time
- `limit=1` - Get latest reading

**Response**:
```json
[
  {
    "id": "uuid",
    "created_at": "2025-08-29T10:30:00Z",
    "location": "garage",
    "type": "temperature",
    "value": 23.4,
    "device_id": "garage_sensor_01",
    "rssi": -45,
    "metadata": {
      "uptime": 3600,
      "free_heap": 50000
    }
  }
]
```

### GET Historical Data
**Endpoint**: `/rest/v1/environment_measurements`
**Query Parameters**:
- `created_at=gte.2025-08-28T00:00:00Z` - Time range filter
- `location=eq.heating` - Location filter
- `order=created_at.desc` - Time ordering

## Data Quality Requirements

### DQ1: Data Validation
- Temperature range: -40°C to +80°C
- Humidity range: 0% to 100%
- Timestamp validation: Not future dated
- Location validation: Must match known locations
- Device ID format: alphanumeric with underscores

### DQ2: Data Completeness
- Maximum gap between readings: 30 minutes
- Sensor health monitoring via last_seen tracking
- Automatic stale data flagging after 1 hour

### DQ3: Data Retention
- Raw data: 1 year retention
- Aggregated hourly data: 5 years
- Daily aggregates: Indefinite

## Real-time Subscriptions

### Supabase Realtime
```typescript
// Subscribe to new measurements
const subscription = supabase
  .from('environment_measurements')
  .on('INSERT', payload => {
    console.log('New measurement:', payload.new);
  })
  .subscribe();
```

## Performance Requirements

### Response Times
- Current readings: < 200ms
- 24-hour historical: < 1s
- 7-day historical: < 2s
- Real-time updates: < 500ms latency

### Scalability
- Support up to 50 concurrent sensors
- Handle 1 measurement per sensor per 5 minutes
- Support 100 concurrent dashboard users

## Error Handling & Resilience

### EH1: Sensor Failures
- Graceful handling of missing data
- Last known value caching
- Sensor offline detection and alerting

### EH2: Network Issues
- Sensor-side data buffering during outages
- Automatic retry mechanisms
- Duplicate detection and handling

### EH3: Database Issues
- Connection pooling and failover
- Query timeout handling
- Backup and recovery procedures

## Security Requirements

### Authentication
- Anonymous read access for dashboard
- Service key for sensor writes
- Rate limiting: 60 requests per minute per IP

### Data Protection
- HTTPS only connections
- Environment variable storage for secrets
- Input sanitization and validation
- SQL injection protection (built into Supabase)

## Monitoring & Alerting

### Application Metrics
- Database connection health
- Query performance monitoring
- Error rate tracking
- Data freshness monitoring

### Business Metrics
- Sensor uptime percentage
- Data collection success rate
- Dashboard user engagement
- System availability (99.9% target)

## Alternative Data Architecture (MQTT + PostgreSQL)

### For Complex Deployments
```
ESP32 → MQTT Broker → Python Bridge → PostgreSQL → API Server → Frontend
```

### MQTT Topics Structure
```
heating/garage/temperature
heating/garage/humidity
heating/boiler_room/temperature
sensors/status/garage_sensor_01
```

### PostgreSQL Schema (Alternative)
```sql
CREATE TABLE temperature_readings (
    id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    device_id VARCHAR(50) NOT NULL,
    location VARCHAR(50) NOT NULL,
    temperature FLOAT NOT NULL,
    humidity FLOAT,
    rssi INTEGER,
    battery_level FLOAT,
    metadata JSONB
);

-- TimescaleDB hypertable for time-series optimization
SELECT create_hypertable('temperature_readings', 'timestamp');
```

## Data Migration & Import

### Historical Data Import
- CSV import functionality
- Data format validation
- Duplicate detection
- Batch processing capabilities

### Export Capabilities
- CSV export for analysis
- JSON export for backups
- Date range filtering
- Location-specific exports

## Backup & Recovery

### Backup Strategy
- Daily automated backups via Supabase
- Point-in-time recovery capability
- Cross-region backup replication
- Monthly backup testing

### Disaster Recovery
- RTO: 4 hours (Recovery Time Objective)
- RPO: 1 hour (Recovery Point Objective)
- Documented recovery procedures
- Regular disaster recovery testing

## Future Enhancements

### Phase 2 Features
- Data aggregation tables (hourly/daily averages)
- Calculated fields (heat index, dew point)
- Advanced analytics endpoints
- Predictive modeling data preparation

### Phase 3 Features
- Multi-tenant support
- Advanced alerting system
- Data streaming APIs
- Machine learning integration

This specification ensures robust backend support for the temperature monitoring system with scalability and reliability in mind.