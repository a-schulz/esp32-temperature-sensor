#!/usr/bin/env bash

echo "Setting up Heating System IoT Infrastructure..."

# Create directory structure
mkdir -p mosquitto/config mosquitto/data mosquitto/log
mkdir -p homeassistant
mkdir -p influxdb/data influxdb/config
mkdir -p grafana/data grafana/provisioning
mkdir -p node-red

# Set proper permissions for mosquitto
sudo chown -R root:root mosquitto/config
sudo chown -R 1883:1883 mosquitto/data mosquitto/log
sudo chown -R 472:472 grafana/
sudo chown -R 1000:1000 node-red/

# Create MQTT users with proper permissions
echo "Creating MQTT users..."
sudo docker run --rm -v $(pwd)/mosquitto/config:/mosquitto/config eclipse-mosquitto:2.0 mosquitto_passwd -c -b /mosquitto/config/passwd heating_system secure_mqtt_password_2024
sudo docker run --rm -v $(pwd)/mosquitto/config:/mosquitto/config eclipse-mosquitto:2.0 mosquitto_passwd -b /mosquitto/config/passwd admin admin_password_2024

# Fix ownership of password file
sudo chown root:root mosquitto/config/passwd
sudo chmod 644 mosquitto/config/passwd
echo "MQTT users created successfully."

echo "Copy mosquitto configuration..."
sudo cp mosquitto.conf mosquitto/config/mosquitto.conf
sudo chown root:root mosquitto/config/mosquitto.conf
sudo chmod 644 mosquitto/config/mosquitto.conf

# Create ACL file with proper permissions
sudo tee mosquitto/config/acl > /dev/null << EOF
# Admin user
user admin
topic readwrite #

# Heating system user
user heating_system
topic readwrite heating/#
topic readwrite homeassistant/#
EOF

sudo chown root:root mosquitto/config/acl
sudo chmod 644 mosquitto/config/acl

# Create Home Assistant configuration
cat > homeassistant/configuration.yaml << EOF
# Loads default set of integrations
default_config:

# MQTT Configuration
mqtt:
  broker: mosquitto
  port: 1883
  username: heating_system
  password: secure_mqtt_password_2024
  discovery: true

# InfluxDB Integration
influxdb:
  host: influxdb
  port: 8086
  token: super_secret_admin_token_2024
  organization: heating_system
  bucket: temperature_data
  measurement_attr: entity_id
  default_measurement: units
  exclude:
    domains:
      - automation
      - script

# Recorder (for local database)
recorder:
  purge_keep_days: 30
  
# History
history:
  include:
    entities:
      - sensor.boiler_room_temperature
      - sensor.living_room_temperature
      - switch.heating_pump

# Frontend
frontend:
  themes: !include_dir_merge_named themes

# Mobile App
mobile_app:

# System Health
system_health:
EOF

echo "Setup complete! Starting services..."
docker compose up -d

echo "Waiting for services to start..."
sleep 30

echo "Service status:"
docker compose ps

echo ""
echo "Access URLs:"
echo "Home Assistant: http://$(hostname | awk '{print $1}'):8123"
echo "Grafana: http://$(hostname | awk '{print $1}'):3000 (admin/admin_password_2024)"
echo "Node-RED: http://$(hostname | awk '{print $1}'):1880"
echo "InfluxDB: http://$(hostname | awk '{print $1}'):8086"
echo ""
echo "MQTT Broker: $(hostname | awk '{print $1}'):1883"
echo "MQTT User: heating_system"
echo "MQTT Password: secure_mqtt_password_2024"
