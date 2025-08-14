// Update for simplified MQTT (no auth required)
const char* device_id = "heating_sensor_01";
const char* location = "boiler_room"; // Change for each sensor location

// MQTT topics - same format but simpler broker
String temp_topic = "heating/" + String(location) + "/temperature";
String status_topic = "heating/" + String(location) + "/status";

// ...existing code...

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    showStatus("MQTT...");

    String clientId = "ESP32-" + String(device_id);

    // Simple connection - no username/password for simplified setup
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
      publishStatus("online", -999);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ...existing code...

void setup() {
  // ...existing code...

  // Setup MQTT - just use IP address, no auth
  client.setServer(MQTT_SERVER, 1883);

  // ...existing code...
}
