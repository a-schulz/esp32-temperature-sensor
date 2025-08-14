#!/usr/bin/env bash
set -Eeuo pipefail

# Load environment variables
if [ -f .env ]; then
  # Export all variables defined in .env, ignore comments
  set -a
  . ./.env
  set +a
else
  echo "Error: .env file not found. Please copy .env.example to .env and configure your settings." >&2
  exit 1
fi

# Choose docker compose command
if command -v docker-compose >/dev/null 2>&1; then
  COMPOSE="docker-compose"
else
  COMPOSE="docker compose"
fi

echo "Setting up Heating System IoT Infrastructure with TimescaleDB..."

# Create directory structure
sudo mkdir -p mosquitto/config mosquitto/data mosquitto/log
sudo mkdir -p homeassistant
sudo mkdir -p timescaledb/data
sudo mkdir -p grafana/data
sudo mkdir -p node-red

# Align TimescaleDB data dir ownership to container postgres UID/GID (prevents permission denied on data files)
echo "Aligning TimescaleDB data directory ownership..."
PG_UID=$(docker run --rm timescale/timescaledb:latest-pg17 id -u postgres 2>/dev/null || echo "")
PG_GID=$(docker run --rm timescale/timescaledb:latest-pg17 id -g postgres 2>/dev/null || echo "")
if [ -n "${PG_UID}" ] && [ -n "${PG_GID}" ]; then
  sudo chown -R "${PG_UID}:${PG_GID}" timescaledb/data || true
else
  echo "Warning: Could not determine postgres UID/GID from image; skipping chown." >&2
fi

# Set proper permissions (let the DB container manage its own data directory ownership)
sudo chown -R root:root mosquitto/config || true
sudo chown -R 1883:1883 mosquitto/data mosquitto/log || true
sudo chown -R 472:472 grafana/ || true
sudo chown -R 1000:1000 node-red/ || true
# Do NOT chown timescaledb/ to a fixed UID to avoid mismatches with image UID

# Create MQTT users with environment variables
echo "Creating MQTT users..."
sudo docker run --rm -v "$(pwd)/mosquitto/config:/mosquitto/config" eclipse-mosquitto:2.0 \
  mosquitto_passwd -c -b /mosquitto/config/passwd "${MQTT_USER}" "${MQTT_PASSWORD}"
sudo docker run --rm -v "$(pwd)/mosquitto/config:/mosquitto/config" eclipse-mosquitto:2.0 \
  mosquitto_passwd -b /mosquitto/config/passwd admin "${MQTT_ADMIN_PASSWORD}"

# Fix ownership of password file
sudo chown root:root mosquitto/config/passwd || true
sudo chmod 644 mosquitto/config/passwd || true

# Create ACL file with environment variables
sudo tee mosquitto/config/acl > /dev/null << EOF
# Admin user
user admin
topic readwrite #

# Heating system user
user ${MQTT_USER}
topic readwrite heating/#
topic readwrite homeassistant/#
EOF

sudo chown root:root mosquitto/config/acl || true
sudo chmod 644 mosquitto/config/acl || true

# Generate Home Assistant configuration from template
echo "Generating Home Assistant configuration..."
if command -v envsubst >/dev/null 2>&1; then
  envsubst < homeassistant/configuration.yaml.template > homeassistant/configuration.yaml
else
  echo "Warning: envsubst not found. Copying template without substitution." >&2
  cp homeassistant/configuration.yaml.template homeassistant/configuration.yaml
fi

# Create empty automation files if they don't exist
: > homeassistant/automations.yaml
: > homeassistant/scripts.yaml
: > homeassistant/scenes.yaml

echo "Starting services with docker compose..."
$COMPOSE up -d

echo "Waiting for services to become healthy..."
# Wait up to 180s for DB readiness
DB_READY_TIMEOUT=180
DB_READY_START=$(date +%s)
while true; do
  if docker exec timescaledb pg_isready -U "${POSTGRES_USER}" -d "${POSTGRES_DB}" >/dev/null 2>&1; then
    break
  fi
  NOW=$(date +%s)
  if [ $((NOW - DB_READY_START)) -ge ${DB_READY_TIMEOUT} ]; then
    echo "Warning: TimescaleDB not ready after ${DB_READY_TIMEOUT}s; continuing." >&2
    break
  fi
  sleep 3
done

# Also give HA some time to create tables
sleep 20

echo "Service status:"
$COMPOSE ps

echo ""
echo "Initializing TimescaleDB hypertables..."
# Attempt to create hypertables and views (safe to run multiple times)
docker exec timescaledb psql -U "${POSTGRES_USER}" -d "${POSTGRES_DB}" -c "SELECT create_ha_hypertables();" \
  || echo "Note: Hypertables will be created after Home Assistant starts and creates tables"

IP=$(hostname -I 2>/dev/null | awk '{print $1}')
[ -z "$IP" ] && IP=$(hostname)

echo ""
echo "Setup complete!"
echo ""
echo "Access URLs:"
echo "Home Assistant: http://$IP:8123"
echo "Grafana: http://$IP:3000 (admin/${GRAFANA_ADMIN_PASSWORD})"
echo "Node-RED: http://$IP:1880"
echo ""
echo "Database Info:"
echo "TimescaleDB: $IP:5432"
echo "Database: ${POSTGRES_DB}"
echo "User: ${POSTGRES_USER}"
echo ""
echo "MQTT Broker: $IP:1883"
echo "MQTT User: ${MQTT_USER}"
echo ""
echo "To create hypertables manually after Home Assistant has started:"
echo "docker exec timescaledb psql -U ${POSTGRES_USER} -d ${POSTGRES_DB} -c \"SELECT create_ha_hypertables();\""
