# Home Automation Infrastructure

This directory contains **TWO** complete Docker Compose stacks for IoT home automation:

1. **Original Complex Setup** (`docker-compose.yml`) - Full home automation with TimescaleDB
2. **Simplified Setup** (`docker-compose-simple.yml`) - Streamlined temperature monitoring

## 🏗️ Architecture Comparison

### Original Complex Setup (6 Services)
```
ESP32 → MQTT → Home Assistant → TimescaleDB → Grafana
                     ↓
               Node-RED (automation)
```
- **TimescaleDB**: Time-series database with hypertables
- **MQTT (Mosquitto)**: Message broker with authentication
- **Home Assistant**: Full home automation platform with device discovery
- **Grafana**: Data visualization and dashboards  
- **Node-RED**: Visual programming for automation flows
- **Init Scripts**: Complex database setup with multiple users/roles

### Simplified Setup (4 Services)  
```
ESP32 → MQTT → Python Bridge → PostgreSQL → Grafana
```
- **PostgreSQL**: Simple database with one table
- **MQTT (Mosquitto)**: Message broker without authentication
- **Python Bridge**: Direct MQTT-to-database connector
- **Grafana**: Data visualization

---

## 🚀 Quick Start - Choose Your Setup

### Option A: Simplified Setup (Recommended for Temperature Monitoring)

**Use this if you want:**
- Simple temperature logging
- Easy to understand and debug
- Minimal resource usage
- Quick setup

```bash
# 1. Start the simplified stack
./setup-simple.sh

# 2. Update your ESP32 code to use simple MQTT (no auth)
# Upload src/main_ds18b20_mqtt_simple.cpp

# 3. Access Grafana
# http://your-pi-ip:3000 (admin/admin123)
```

**Services running:**
- MQTT Broker: `localhost:1883` (no authentication)
- PostgreSQL: `localhost:5432` (sensor_user/sensor123)
- Grafana: `localhost:3000` (admin/admin123)
- Python Bridge: Automatically connects MQTT to database

### Option B: Original Complex Setup (For Full Home Automation)

**Use this if you want:**
- Full home automation platform
- Device discovery and management
- Complex automations with Node-RED
- Enterprise-grade TimescaleDB features

```bash
# 1. Configure environment
cp .env.example .env
nano .env  # Edit with your settings

# 2. Start the complex stack
./setup.sh

# 3. Upload ESP32 code with authentication
# Upload src/main_ds18b20_mqtt.cpp

# 4. Access services
# Home Assistant: http://your-pi-ip:8123
# Grafana: http://your-pi-ip:3000
# Node-RED: http://your-pi-ip:1880
```

**Services running:**
- MQTT Broker: `localhost:1883` (with authentication)
- TimescaleDB: `localhost:5432` (multiple users/roles)
- Home Assistant: `localhost:8123` (full automation platform)
- Grafana: `localhost:3000` 
- Node-RED: `localhost:1880`

---

## 🔧 Setup Instructions

### For Simplified Setup

1. **Run the setup:**
```bash
chmod +x setup-simple.sh
./setup-simple.sh
```

2. **Update ESP32 credentials:**
```cpp
// In include/credentials.h
#define MQTT_SERVER "192.168.1.100"  // Your Pi IP
#define MQTT_PORT 1883
// No MQTT_USER or MQTT_PASSWORD needed
```

3. **Upload simplified ESP32 code:**
```bash
pio run -e ds18b20-mqtt -t upload --upload-port=/dev/ttyACM0
# Use src/main_ds18b20_mqtt_simple.cpp
```

4. **View data in Grafana:**
   - URL: http://your-pi-ip:3000
   - Login: admin/admin123
   - Query: `SELECT timestamp as time, location as metric, temperature as value FROM temperature_readings WHERE $__timeFilter(timestamp)`

### For Original Complex Setup

1. **Environment Configuration**
```bash
# Copy the example environment file
cp .env.example .env

# Edit .env with your specific settings
nano .env
```

2. **Required Environment Variables**
Configure these in your `.env` file.

3. **Deploy the Stack**
```bash
# Make setup script executable
chmod +x setup.sh

# Run setup (creates directories, users, and starts services)
./setup.sh
```

## 📊 Data Flow Comparison

### Simplified Flow
```
ESP32 Sensor
    ↓ (WiFi)
MQTT Broker (no auth)
    ↓ (Python subscriber)
PostgreSQL (temperature_readings table)
    ↓ (SQL query)
Grafana Dashboard
```

### Complex Flow  
```
ESP32 Sensor
    ↓ (WiFi + MQTT auth)
MQTT Broker (authenticated)
    ↓ (MQTT discovery + integration)
Home Assistant (entity management)
    ↓ (Recorder component)
TimescaleDB (states/events/statistics tables)
    ↓ (Complex queries)
Grafana Dashboard + Node-RED Automations
```

---

## 🗂️ File Structure

```
homeAutomation/
├── .env.example                     # Environment variables template
├── docker-compose.yml              # Original complex stack
├── docker-compose-simple.yml       # Simplified stack  
├── setup.sh                        # Complex setup script
├── setup-simple.sh                 # Simplified setup script
├── Dockerfile                      # Python bridge container
├── mqtt_to_db.py                   # MQTT-to-database bridge
├── requirements.txt                # Python dependencies
├── init.sql                        # Simple database schema
├── mosquitto-simple.conf           # MQTT config (no auth)
├── mosquitto.conf                  # MQTT config (with auth)
├── grafana-datasource.yml          # Simple Grafana datasource
├── grafana/
│   └── provisioning/
│       └── datasources/
│           └── timescaledb.yml     # Complex Grafana datasource
├── homeassistant/
│   └── configuration.yaml.template # HA config template
└── timescaledb/
    └── init/
        ├── 00-init-users.sh       # Database user setup
        └── 01-init-timescaledb.sql # TimescaleDB initialization
```

---

## 🔄 Switching Between Setups

### From Complex to Simplified
```bash
# Stop complex stack
docker-compose down

# Start simplified stack  
./setup-simple.sh

# Update ESP32 to use simple MQTT (no auth)
```

### From Simplified to Complex
```bash
# Stop simplified stack
docker-compose -f docker-compose-simple.yml down

# Configure environment
cp .env.example .env && nano .env

# Start complex stack
./setup.sh

# Update ESP32 to use authenticated MQTT
```

---

## Services

### Home Assistant
- **URL**: http://your-pi-ip:8123
- **Database**: Configured to use TimescaleDB for recorder
- **MQTT**: Auto-discovery enabled for ESP32 sensors

### Grafana
- **URL**: http://your-pi-ip:3000
- **Login**: admin / (from GRAFANA_ADMIN_PASSWORD)
- **Data Source**: TimescaleDB (auto-configured)

### MQTT Broker
- **Host**: your-pi-ip:1883
- **Users**: 
  - `heating_system` (for ESP32 devices)
  - `admin` (full access)

### TimescaleDB
- **Host**: your-pi-ip:5432
- **Database**: homeassistant
- **Users**:
  - `homeassistant` (Home Assistant recorder)
  - `grafana` (read-only for Grafana)

### Node-RED
- **URL**: http://your-pi-ip:1880
- **Purpose**: Visual automation flows

## ESP32 Integration

### Update Your ESP32 Credentials
Create `include/credentials.h` with values from your `.env`:

```cpp
#define WIFI_SSID "your-wifi-network"
#define WIFI_PASSWORD "your-wifi-password"
#define MQTT_SERVER "192.168.1.100"  // Your Raspberry Pi IP
#define MQTT_PORT 1883
#define MQTT_USER "heating_system"
#define MQTT_PASSWORD "secure_mqtt_password_2024"
```

### Deploy DS18B20 MQTT Sensor
```bash
cd /home/aschulz/Projects/temperature-sensor
pio run -e ds18b20-mqtt -t upload --upload-port=/dev/ttyACM0
```

## TimescaleDB Features

### Hypertables
Automatically created for optimal time-series performance:
- `states` - Home Assistant entity states
- `events` - Home Assistant events
- `statistics` - Statistical data

### Views
- `temperature_view` - Easy access to temperature sensor data

### Manual Hypertable Creation
If needed after Home Assistant starts:
```bash
docker exec timescaledb psql -U homeassistant -d homeassistant -c "SELECT create_ha_hypertables();"
```

## Grafana Dashboards

### Sample Queries
```sql
-- Current temperatures by location
SELECT 
  last_updated_ts as time,
  location as metric,
  temperature as value
FROM temperature_view
WHERE $__timeFilter(last_updated_ts)
ORDER BY time;

-- Heating efficiency over time
SELECT 
  s1.last_updated_ts as time,
  (s1.state::numeric / NULLIF(s2.state::numeric, 0)) as efficiency
FROM states s1
JOIN states s2 ON s1.last_updated_ts = s2.last_updated_ts
WHERE s1.entity_id = 'sensor.boiler_room_temperature'
  AND s2.entity_id = 'sensor.heating_power_consumption'
  AND $__timeFilter(s1.last_updated_ts)
ORDER BY time;
```

## Maintenance

### View Logs
```bash
# All services
docker-compose logs

# Specific service
docker-compose logs homeassistant
docker-compose logs timescaledb
docker-compose logs mosquitto
```

### Backup Database
```bash
docker exec timescaledb pg_dump -U homeassistant homeassistant > backup.sql
```

### Update Services
```bash
docker-compose pull
docker-compose up -d
```

## Security Notes

- All secrets are stored in `.env` (not committed to git)
- MQTT uses authentication and ACL
- TimescaleDB has separate users with appropriate permissions
- Generated configuration files are excluded from git

## Troubleshooting

### MQTT Connection Issues
```bash
# Test MQTT connection
docker exec mosquitto mosquitto_pub -h localhost -u heating_system -P your_password -t test -m "hello"
```

### Database Connection Issues
```bash
# Test database connection
docker exec timescaledb psql -U homeassistant -d homeassistant -c "SELECT version();"
```

### Home Assistant Database Issues
Check if TimescaleDB URL is correct in configuration.yaml and restart Home Assistant.

### Permission Issues
Re-run the setup script to fix directory permissions:
```bash
./setup.sh
```

# On Raspberry Pi
chmod +x setup.sh
./setup.sh

# Upload ESP32 code
pio run -e ds18b20-mqtt -t upload --upload-port=/dev/ttyACM0
```
