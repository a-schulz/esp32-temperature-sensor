#!/usr/bin/env bash
set -e

echo "🚀 Starting Simple IoT Temperature Monitoring Stack"
echo ""

# Copy simple environment file if .env doesn't exist
if [ ! -f .env ]; then
    cp .env-simple .env
    echo "✅ Created .env file with default settings"
fi

# Load environment variables
set -a
. ./.env
set +a

# Create data directories
mkdir -p postgres_data grafana_data

echo "🐳 Starting services..."
docker-compose -f docker-compose-simple.yml up -d --build

echo ""
echo "⏳ Waiting for services to start..."
sleep 15

echo ""
echo "🎯 Testing the data flow:"

# Test MQTT publishing
echo "📡 Publishing test temperature..."
docker exec mosquitto mosquitto_pub -h localhost -t "heating/boiler_room/temperature" -m "23.5"

sleep 2

# Check database
echo "🗄️  Checking database..."
docker exec postgres psql -U sensor_user -d sensors -c "SELECT location, temperature, timestamp FROM temperature_readings ORDER BY timestamp DESC LIMIT 3;"

echo ""
echo "✨ Setup complete! Here's what's running:"
echo ""
echo "🌡️  Your ESP32 sends data to: MQTT broker at localhost:1883"
echo "🔄 mqtt-to-db service automatically stores MQTT data in PostgreSQL"
echo "📊 Grafana dashboard: http://localhost:3000 (admin/admin123)"
echo "🗄️  PostgreSQL: localhost:5432 (sensor_user/sensor123)"
echo ""
echo "📈 To view your data in Grafana:"
echo "   1. Go to http://localhost:3000"
echo "   2. Login with admin/admin123"
echo "   3. Create a new dashboard"
echo "   4. Use this query: SELECT timestamp as time, location as metric, temperature as value FROM temperature_readings WHERE \$__timeFilter(timestamp)"
echo ""
echo "🔧 To see live MQTT data:"
echo "   docker exec mosquitto mosquitto_sub -h localhost -t 'heating/+/+'"
echo ""
echo "📱 ESP32 Update: Change these in your credentials.h:"
echo "   #define MQTT_SERVER \"$(hostname -I | awk '{print $1}')\""
echo "   #define MQTT_PORT 1883"
echo "   // No username/password needed for this simple setup"
