// Storing local measurements in supabase and displaying on low-power OLED while listening to a button to toggle display


#include <Arduino.h>
#include <WiFi.h>
#include <ESPSupabase.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include "credentials.h"

// ========== DISPLAY CONFIGURATION ==========
#define SDA_PIN 5
#define SCL_PIN 6

// ========== BOOT BUTTON CONFIGURATION ==========
#define BOOT_BUTTON_PIN 9  // GPIO9 is the boot button on ESP32-C3

// Setup U8g2 display object for ESP32-C3 OLED (72x40)
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ========== POWER-EFFICIENT CONFIGURATION ==========
#define WIFI_CONNECT_TIMEOUT 15000     // Reduce WiFi timeout to 15 seconds
#define DATA_UPDATE_INTERVAL 300000    // Update data every 5 minutes (300,000ms)
#define DISPLAY_UPDATE_INTERVAL 30000  // Update display every 30 seconds
#define BUTTON_DEBOUNCE_TIME 200       // Button debounce time in ms

// ========== SUPABASE OBJECTS ==========
Supabase supabase;

// ========== STATE VARIABLES ==========
float lastTemperature = -999.0;
unsigned long lastDataUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastButtonPress = 0;
bool displayOn = true;
bool wifiConnected = false;
int updateCount = 0;
String lastUpdateTime = "";

// ========== FUNCTION DECLARATIONS ==========
void setupPowerSaving();
void initializeDisplay();
void setupBootButton();
bool checkBootButton();
bool setupWiFi();
void updateDisplay();
float queryLatestTemperature();
void updateTemperatureData();
void printSystemInfo();

// ========== POWER MANAGEMENT ==========
void setupPowerSaving() {
  // Reduce CPU frequency to save power
  setCpuFrequencyMhz(80); // Reduce from 160MHz to 80MHz
  
  // Configure WiFi power saving
  WiFi.setSleep(true);
  
  Serial.println("Power saving configured: CPU 80MHz, WiFi sleep enabled");
}

void initializeDisplay() {
  Serial.println("Initializing OLED display...");
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  
  // Show startup message
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Food Storage");
  u8g2.drawStr(0, 20, "Monitor v2");
  u8g2.drawStr(0, 30, "Starting...");
  u8g2.sendBuffer();
  delay(2000);
}

void setupBootButton() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Boot button configured for display toggle");
}

bool checkBootButton() {
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonPress > BUTTON_DEBOUNCE_TIME) {
      lastButtonPress = currentTime;
      displayOn = !displayOn;
      
      if (displayOn) {
        u8g2.setPowerSave(0); // Turn display on
        Serial.println("Display turned ON");
        // Immediately update display
        updateDisplay();
      } else {
        u8g2.clearBuffer();
        u8g2.sendBuffer();
        u8g2.setPowerSave(1); // Turn display off
        Serial.println("Display turned OFF");
      }
      
      return true;
    }
  }
  return false;
}

bool setupWiFi() {
  Serial.println("=== WiFi Connection Debug ===");
  Serial.printf("SSID: %s\n", WIFI_SSID);
  Serial.printf("Password length: %d\n", strlen(WIFI_PASSWORD));
  
  // Disconnect any previous connection and clear stored credentials
  WiFi.disconnect(true);
  delay(1000);
  
  // Set WiFi mode and disable power saving during connection
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Disable sleep during connection attempt
  
  Serial.println("Scanning for available networks...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found!");
  } else {
    Serial.printf("Found %d networks:\n", n);
    bool ssidFound = false;
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      wifi_auth_mode_t encType = WiFi.encryptionType(i);
      
      Serial.printf("  %d: %s (RSSI: %d, Auth: %d)\n", 
                    i, ssid.c_str(), rssi, encType);
      
      if (ssid.equals(WIFI_SSID)) {
        ssidFound = true;
        Serial.printf("✓ Target SSID found with signal strength: %d dBm\n", rssi);
      }
    }
    
    if (!ssidFound) {
      Serial.println("✗ Target SSID not found in scan!");
      return false;
    }
  }
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Show connection progress on display
  if (displayOn) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "Connecting");
    u8g2.drawStr(0, 20, "to WiFi...");
    u8g2.sendBuffer();
  }
  
  unsigned long startAttemptTime = millis();
  int dotCount = 0;
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_CONNECT_TIMEOUT) {
    delay(500);
    Serial.print(".");
    dotCount++;
    
    // Update display with progress dots
    if (displayOn && dotCount % 4 == 0) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 10, "Connecting");
      
      char progress[20];
      sprintf(progress, "WiFi%s", String('.').c_str());
      for (int i = 0; i < (dotCount / 4) % 4; i++) {
        strcat(progress, ".");
      }
      u8g2.drawStr(0, 20, progress);
      u8g2.sendBuffer();
    }
    
    // Print detailed status every 5 seconds
    if ((millis() - startAttemptTime) % 5000 == 0) {
      wl_status_t status = WiFi.status();
      Serial.printf("\nWiFi Status: %d ", status);
      switch (status) {
        case WL_IDLE_STATUS: Serial.println("(Idle)"); break;
        case WL_NO_SSID_AVAIL: Serial.println("(SSID not available)"); break;
        case WL_SCAN_COMPLETED: Serial.println("(Scan completed)"); break;
        case WL_CONNECTED: Serial.println("(Connected)"); break;
        case WL_CONNECT_FAILED: Serial.println("(Connect failed)"); break;
        case WL_CONNECTION_LOST: Serial.println("(Connection lost)"); break;
        case WL_DISCONNECTED: Serial.println("(Disconnected)"); break;
        default: Serial.println("(Unknown)"); break;
      }
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected successfully!");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
    Serial.printf("MAC address: %s\n", WiFi.macAddress().c_str());
    
    // Show success on display
    if (displayOn) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 10, "WiFi OK!");
      u8g2.drawStr(0, 20, WiFi.localIP().toString().c_str());
      u8g2.sendBuffer();
      delay(2000);
    }
    
    // Re-enable power saving after successful connection
    WiFi.setSleep(true);
    
    return true;
  } else {
    wl_status_t finalStatus = WiFi.status();
    Serial.println("\n✗ Failed to connect to WiFi");
    Serial.printf("Final status: %d ", finalStatus);
    
    switch (finalStatus) {
      case WL_NO_SSID_AVAIL:
        Serial.println("(SSID not available - check network name)");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("(Connection failed - check signal strength)");
        break;
      default:
        Serial.println("(Connection timeout)");
        break;
    }
    
    // Show failure on display
    if (displayOn) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 10, "WiFi Failed");
      u8g2.drawStr(0, 20, "Check signal");
      u8g2.drawStr(0, 30, "or password");
      u8g2.sendBuffer();
      delay(3000);
    }
    
    return false;
  }
}

void updateDisplay() {
  if (!displayOn) return;
  
  u8g2.clearBuffer();
  
  // Display location header
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 8, "Food Store");
  
  if (lastTemperature != -999.0) {
    // Display temperature with large font
    u8g2.setFont(u8g2_font_logisoso18_tf);
    char tempStr[10];
    sprintf(tempStr, "%.1f", lastTemperature);
    u8g2.drawStr(0, 28, tempStr);
    
    // Add degree symbol and C
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(50, 22, "C");
  } else {
    // No data available
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 18, "No Data");
    u8g2.drawStr(0, 28, "Available");
  }
  
  // Status indicators
  u8g2.setFont(u8g2_font_5x7_tr);
  
  // WiFi status
  if (wifiConnected) {
    u8g2.drawStr(60, 8, "W");
  }
  
  // Update count
  char countStr[10];
  sprintf(countStr, "#%d", updateCount);
  u8g2.drawStr(0, 40, countStr);
  
  // Last update time (show minutes ago)
  unsigned long minutesAgo = (millis() - lastDataUpdate) / 60000;
  char timeStr[10];
  if (minutesAgo < 60) {
    sprintf(timeStr, "%lum", minutesAgo);
  } else {
    sprintf(timeStr, "%luh", minutesAgo / 60);
  }
  u8g2.drawStr(35, 40, timeStr);
  
  u8g2.sendBuffer();
}

float queryLatestTemperature() {
  if (!wifiConnected) {
    Serial.println("WiFi not connected, cannot query Supabase");
    return -999.0;
  }
  
  Serial.println("Querying Supabase for latest food_storage temperature...");
  
  // Query for the latest temperature reading from food_storage location
  String response = supabase.from("environment_measurements")
                      .select("value, created_at")
                      .eq("location", "food_storage")
                      .eq("type", "temperature")
                      .order("created_at", "desc", false)
                      .limit(1)
                      .doSelect();
  
  Serial.print("Supabase response: ");
  Serial.println(response);
  
  if (response.length() > 0 && !response.startsWith("error")) {
    // Parse JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error && doc.is<JsonArray>() && doc.size() > 0) {
      JsonObject measurement = doc[0];
      if (measurement.containsKey("value")) {
        float temperature = measurement["value"].as<float>();
        Serial.printf("Retrieved temperature: %.2f°C\n", temperature);
        
        // Update timestamp
        lastDataUpdate = millis();
        updateCount++;
        
        return temperature;
      } else {
        Serial.println("No 'value' field in response");
      }
    } else {
      Serial.println("Failed to parse JSON response or empty result");
      if (error) {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
      }
    }
  } else {
    Serial.println("Empty or error response from Supabase");
  }
  
  return -999.0; // Error value
}

void updateTemperatureData() {
  if (!wifiConnected) return;
  
  float newTemperature = queryLatestTemperature();
  if (newTemperature != -999.0) {
    lastTemperature = newTemperature;
    Serial.println("✓ Temperature data updated successfully!");
  } else {
    Serial.println("⚠ Failed to update temperature data");
  }
}

void printSystemInfo() {
  Serial.println("=== Food Storage Monitor v2 ===");
  Serial.printf("Update Count: %d\n", updateCount);
  Serial.printf("Last Temperature: %.1f°C\n", lastTemperature);
  Serial.printf("Data Update Interval: %d seconds\n", DATA_UPDATE_INTERVAL / 1000);
  Serial.printf("Display Update Interval: %d seconds\n", DISPLAY_UPDATE_INTERVAL / 1000);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
  Serial.println("Boot button: Press to toggle display");
  Serial.println("===============================");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Print system information
  printSystemInfo();
  
  // Setup power saving
  setupPowerSaving();
  
  // Initialize display
  initializeDisplay();
  
  // Setup boot button
  setupBootButton();
  
  // Connect to WiFi
  wifiConnected = setupWiFi();
  
  if (wifiConnected) {
    // Initialize Supabase connection
    Serial.println("Initializing Supabase connection...");
    supabase.begin(SUPABASE_URL, SUPABASE_KEY);
    
    // Get initial data
    updateTemperatureData();
  } else {
    Serial.println("⚠ No WiFi connection - will display cached data only");
  }
  
  // Initial display update
  lastDisplayUpdate = millis();
  updateDisplay();
  
  Serial.println("=== Food Storage Monitor Started ===");
  Serial.println("Press boot button to toggle display on/off");
  Serial.println("Updates: Data every 5min, Display every 30sec");
  Serial.println("====================================");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check boot button for display toggle
  checkBootButton();
  
  // Update temperature data every 5 minutes
  if (wifiConnected && (currentTime - lastDataUpdate >= DATA_UPDATE_INTERVAL)) {
    Serial.println("=== Scheduled Data Update ===");
    updateTemperatureData();
  }
  
  // Update display every 30 seconds (if display is on)
  if (displayOn && (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)) {
    updateDisplay();
    lastDisplayUpdate = currentTime;
  }
  
  // Check WiFi status periodically and attempt reconnection
  if (!wifiConnected && (currentTime % 60000 == 0)) { // Check every minute
    Serial.println("Attempting WiFi reconnection...");
    wifiConnected = setupWiFi();
    if (wifiConnected) {
      supabase.begin(SUPABASE_URL, SUPABASE_KEY);
      updateTemperatureData();
    }
  }
  
  // Small delay to prevent excessive CPU usage
  delay(50);
}