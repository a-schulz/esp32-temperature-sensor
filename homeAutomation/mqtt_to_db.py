#!/usr/bin/env python3
"""
Simple MQTT to PostgreSQL bridge
Listens to heating sensor data and stores it in the database
"""

import json
import time
import os
import logging
from datetime import datetime
import paho.mqtt.client as mqtt
import psycopg2
from psycopg2.extras import RealDictCursor

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Configuration from environment
MQTT_HOST = os.getenv('MQTT_HOST', 'mosquitto')
MQTT_PORT = int(os.getenv('MQTT_PORT', 1883))
POSTGRES_HOST = os.getenv('POSTGRES_HOST', 'postgres')
POSTGRES_DB = os.getenv('POSTGRES_DB', 'sensors')
POSTGRES_USER = os.getenv('POSTGRES_USER', 'sensor_user')
POSTGRES_PASSWORD = os.getenv('POSTGRES_PASSWORD', 'sensor123')

class MQTTToDatabaseBridge:
    def __init__(self):
        self.mqtt_client = None
        self.db_connection = None
        self.setup_database()
        self.setup_mqtt()

    def setup_database(self):
        """Initialize database connection"""
        try:
            self.db_connection = psycopg2.connect(
                host=POSTGRES_HOST,
                database=POSTGRES_DB,
                user=POSTGRES_USER,
                password=POSTGRES_PASSWORD
            )
            logger.info("Connected to PostgreSQL database")
        except Exception as e:
            logger.error(f"Failed to connect to database: {e}")
            raise

    def setup_mqtt(self):
        """Initialize MQTT client"""
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_message = self.on_message
        self.mqtt_client.on_disconnect = self.on_disconnect

    def on_connect(self, client, userdata, flags, rc):
        """Called when MQTT client connects"""
        if rc == 0:
            logger.info("Connected to MQTT broker")
            # Subscribe to all heating sensor topics
            client.subscribe("heating/+/temperature")
            client.subscribe("heating/+/status")
        else:
            logger.error(f"Failed to connect to MQTT broker: {rc}")

    def on_disconnect(self, client, userdata, rc):
        """Called when MQTT client disconnects"""
        logger.warning(f"Disconnected from MQTT broker: {rc}")

    def on_message(self, client, userdata, msg):
        """Process incoming MQTT messages"""
        try:
            topic = msg.topic
            payload = msg.payload.decode('utf-8')

            logger.info(f"Received: {topic} -> {payload}")

            # Parse topic: heating/location/type
            topic_parts = topic.split('/')
            if len(topic_parts) != 3 or topic_parts[0] != 'heating':
                return

            location = topic_parts[1]
            data_type = topic_parts[2]

            if data_type == 'temperature':
                # Simple temperature value
                temperature = float(payload)
                self.store_temperature(location, temperature)

            elif data_type == 'status':
                # JSON status message
                status_data = json.loads(payload)
                if 'temperature' in status_data and status_data['temperature'] != -999:
                    self.store_temperature_with_metadata(
                        location,
                        status_data['temperature'],
                        status_data.get('device_id'),
                        status_data.get('rssi'),
                        status_data.get('uptime'),
                        status_data.get('free_heap')
                    )

        except Exception as e:
            logger.error(f"Error processing message {topic}: {e}")

    def store_temperature(self, location, temperature):
        """Store simple temperature reading"""
        try:
            with self.db_connection.cursor() as cursor:
                cursor.execute("""
                    INSERT INTO temperature_readings (location, temperature, device_id)
                    VALUES (%s, %s, %s)
                """, (location, temperature, f"sensor_{location}"))
                self.db_connection.commit()
                logger.info(f"Stored temperature: {location} = {temperature}°C")
        except Exception as e:
            logger.error(f"Failed to store temperature: {e}")
            self.db_connection.rollback()

    def store_temperature_with_metadata(self, location, temperature, device_id, rssi, uptime, free_heap):
        """Store temperature with additional sensor metadata"""
        try:
            with self.db_connection.cursor() as cursor:
                cursor.execute("""
                    INSERT INTO temperature_readings
                    (location, temperature, device_id, rssi, battery_level, metadata)
                    VALUES (%s, %s, %s, %s, %s, %s)
                """, (
                    location,
                    temperature,
                    device_id or f"sensor_{location}",
                    rssi,
                    None,  # battery_level - not available from ESP32 yet
                    json.dumps({"uptime": uptime, "free_heap": free_heap})
                ))
                self.db_connection.commit()
                logger.info(f"Stored temperature with metadata: {location} = {temperature}°C")
        except Exception as e:
            logger.error(f"Failed to store temperature with metadata: {e}")
            self.db_connection.rollback()

    def run(self):
        """Main loop"""
        try:
            # Connect to MQTT broker
            self.mqtt_client.connect(MQTT_HOST, MQTT_PORT, 60)

            # Start MQTT loop
            self.mqtt_client.loop_start()

            logger.info("MQTT to Database bridge is running...")

            # Keep running
            while True:
                time.sleep(1)

        except KeyboardInterrupt:
            logger.info("Shutting down...")
        except Exception as e:
            logger.error(f"Unexpected error: {e}")
        finally:
            self.mqtt_client.loop_stop()
            self.mqtt_client.disconnect()
            if self.db_connection:
                self.db_connection.close()

if __name__ == "__main__":
    bridge = MQTTToDatabaseBridge()
    bridge.run()
