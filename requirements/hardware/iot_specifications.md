# Hardware & IoT Requirements

## Overview
Define the hardware specifications, sensor deployment requirements, and IoT device management for the temperature monitoring system.

## Current Hardware Inventory

### ESP32 Devices
- **ESP32-C3 DevKit**: Primary development boards
- **ESP32 Dev Module**: Standard ESP32 boards
- **Quantity**: Multiple units available
- **Power**: USB or battery powered options

### Sensors Currently Deployed
- **DHT11**: Temperature and humidity sensor
  - Accuracy: ±2°C temperature, ±5% humidity
  - Operating range: -40°C to +80°C, 5% to 95% RH
  - Interface: Single digital pin
  
- **DS18B20**: High-precision temperature sensor
  - Accuracy: ±0.5°C
  - Operating range: -55°C to +125°C
  - Interface: One-Wire digital
  - Waterproof probe available

### Display Hardware
- **OLED Display**: SSD1306 72x40 pixels
- **Interface**: I2C
- **Purpose**: Local temperature display on sensor units

## Deployment Locations

### Current Locations
1. **Garage**
   - Sensor Type: DHT11 (temperature + humidity)
   - Device ID: `garage_sensor_01`
   - Power: USB powered
   - Connectivity: WiFi
   - Purpose: Monitor vehicle storage conditions

2. **Heating System**
   - Sensor Type: DS18B20 (temperature only)
   - Device ID: `heating_sensor_01`
   - Power: USB powered
   - Connectivity: WiFi
   - Purpose: Monitor heating system efficiency

### Future Expansion Locations
3. **Living Room**
   - Sensor Type: DHT11
   - Purpose: Comfort monitoring
   
4. **Bedroom**
   - Sensor Type: DHT11
   - Purpose: Sleep environment optimization

5. **Basement**
   - Sensor Type: DHT11
   - Purpose: Humidity monitoring for mold prevention

## Hardware Requirements

### HR1: Environmental Specifications
- **Operating Temperature**: -20°C to +60°C
- **Humidity Range**: 10% to 90% non-condensing
- **IP Rating**: IP20 minimum (indoor use)
- **Enclosure**: Plastic housing with ventilation holes

### HR2: Power Requirements
- **Primary Power**: 5V USB (wall adapter)
- **Backup Power**: 3.7V Li-ion battery (optional)
- **Power Consumption**: < 500mA average
- **Deep Sleep**: < 50µA during sleep cycles

### HR3: Connectivity
- **Primary**: WiFi 802.11 b/g/n (2.4GHz)
- **Range**: 50m minimum from router
- **Security**: WPA2/WPA3 support
- **Fallback**: Store data locally during outages

### HR4: Data Collection
- **Sampling Rate**: Every 5-15 minutes (configurable)
- **Data Transmission**: Real-time via WiFi
- **Local Storage**: 24 hours of readings (fallback)
- **Time Sync**: NTP synchronization

## Sensor Specifications

### Temperature Sensors
| Sensor | Accuracy | Range | Interface | Cost | Use Case |
|--------|----------|-------|-----------|------|----------|
| DHT11 | ±2°C | -40 to +80°C | Digital | Low | General purpose |
| DHT22 | ±0.5°C | -40 to +80°C | Digital | Medium | Precision indoor |
| DS18B20 | ±0.5°C | -55 to +125°C | One-Wire | Medium | High precision |
| SHT30 | ±0.3°C | -40 to +125°C | I2C | High | Laboratory grade |

### Humidity Sensors
| Sensor | Accuracy | Range | Interface | Notes |
|--------|----------|-------|-----------|--------|
| DHT11 | ±5% | 20-80% RH | Digital | Basic accuracy |
| DHT22 | ±2% | 0-100% RH | Digital | Better accuracy |
| SHT30 | ±2% | 0-100% RH | I2C | Professional grade |

## Device Management

### DM1: Configuration Management
- **WiFi Credentials**: Stored in flash memory
- **Device ID**: Unique identifier per location
- **Sampling Interval**: Configurable via web interface
- **Sensor Calibration**: Manual offset adjustment

### DM2: Firmware Management
- **OTA Updates**: Over-the-air firmware updates
- **Version Control**: Semantic versioning
- **Rollback**: Automatic rollback on failed updates
- **Staging**: Test updates on development devices

### DM3: Remote Monitoring
- **Health Monitoring**: Uptime, memory usage, signal strength
- **Error Reporting**: Automatic error log transmission
- **Diagnostics**: Remote diagnostic commands
- **Restart**: Remote device restart capability

## Installation Requirements

### IR1: Physical Installation
- **Mounting**: Wall mount or shelf placement
- **Height**: 1.5-2m above floor for accurate readings
- **Airflow**: Avoid direct sunlight, heat sources, drafts
- **Accessibility**: Easy access for maintenance

### IR2: Network Setup
- **WiFi Coverage**: Strong signal at installation location
- **Network Access**: Internet connectivity required
- **Firewall**: Outbound HTTPS access to Supabase
- **DNS**: Public DNS resolution capability

### IR3: Power Supply
- **Outlet Access**: Nearby power outlet required
- **Cable Management**: Clean cable routing
- **Surge Protection**: Surge protector recommended
- **UPS**: Uninterruptible power supply for critical locations

## Data Transmission Protocol

### Primary: HTTPS to Supabase
```json
{
  "location": "garage",
  "type": "temperature",
  "value": 23.4,
  "device_id": "garage_sensor_01",
  "rssi": -45,
  "metadata": {
    "uptime": 3600,
    "free_heap": 50000,
    "wifi_channel": 6
  }
}
```

### Alternative: MQTT Protocol
```
Topic: sensors/garage/temperature
Payload: {"value": 23.4, "timestamp": "2025-08-29T10:30:00Z"}

Topic: sensors/garage/status
Payload: {"online": true, "rssi": -45, "uptime": 3600}
```

## Security Requirements

### Device Security
- **Encryption**: All data transmission encrypted
- **Authentication**: Device certificates for MQTT
- **Firmware**: Signed firmware updates only
- **Secrets**: Secure storage of credentials

### Network Security
- **Isolation**: IoT VLAN separation (recommended)
- **Access Control**: MAC address filtering
- **Monitoring**: Network traffic monitoring
- **Updates**: Regular security updates

## Maintenance & Support

### Preventive Maintenance
- **Monthly**: Visual inspection and cleaning
- **Quarterly**: Calibration check against reference
- **Annually**: Battery replacement (if applicable)
- **As needed**: Firmware updates

### Troubleshooting
- **Connectivity Issues**: WiFi signal strength check
- **Data Issues**: Sensor cleaning and recalibration
- **Power Issues**: Power supply voltage check
- **Hardware Issues**: Component replacement

### Support Documentation
- **Installation Guide**: Step-by-step setup instructions
- **Troubleshooting Guide**: Common issues and solutions
- **API Documentation**: Developer reference
- **User Manual**: End-user operation guide

## Quality Assurance

### Testing Requirements
- **Factory Testing**: All devices tested before deployment
- **Field Testing**: 24-hour burn-in at installation site
- **Calibration**: Comparison with certified reference sensors
- **Environmental Testing**: Temperature and humidity cycling

### Acceptance Criteria
- **Accuracy**: Within sensor specification tolerances
- **Reliability**: 99.5% uptime over 30 days
- **Connectivity**: 99% successful data transmissions
- **Response Time**: Data available within 60 seconds

## Cost Analysis

### Per-Location Hardware Cost
- ESP32-C3 DevKit: $10
- DHT11 Sensor: $3
- DS18B20 Sensor: $5
- OLED Display: $8
- Enclosure: $15
- Power Supply: $10
- Cables/Connectors: $5
- **Total per location**: $56 (DHT11) / $58 (DS18B20)

### Annual Operating Costs
- Power consumption: $5 per device per year
- Internet connectivity: Included in existing plan
- Cloud services: $0 (Supabase free tier)
- Maintenance: $20 per device per year

## Scalability Planning

### Short-term (6 months)
- Support 5-10 sensor locations
- Basic monitoring and alerting
- Single WiFi network deployment

### Medium-term (1 year)
- Support 20+ sensor locations
- Advanced analytics and trends
- Multiple building deployment

### Long-term (2+ years)
- Support 50+ sensor locations
- Machine learning integration
- Commercial deployment ready

This hardware specification ensures reliable, cost-effective temperature monitoring with room for future expansion and enhancement.