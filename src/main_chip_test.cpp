#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Set up the LED pin
  pinMode(2, OUTPUT);
  
  Serial.println("=================================");
  Serial.println("ESP32-C3 OLED Board Test");
  Serial.println("=================================");
  
  // Basic chip information
  Serial.print("ESP32 Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Number of cores: ");
  Serial.println(ESP.getChipCores());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  
  // Flash information
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("Flash Speed: ");
  Serial.print(ESP.getFlashChipSpeed() / 1000000);
  Serial.println(" MHz");
  
  // Memory information
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("Total Heap: ");
  Serial.print(ESP.getHeapSize());
  Serial.println(" bytes");
  
  Serial.println("=================================");
  Serial.println("Board is working correctly!");
  Serial.println("LED blink test starting...");
}

void loop() {
  // Blink the built-in LED (usually on GPIO2 for ESP32-C3)
  digitalWrite(2, HIGH);
  Serial.println("LED ON");
  delay(1000);
  
  digitalWrite(2, LOW);
  Serial.println("LED OFF");
  delay(1000);
  
  // Print uptime every loop
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
}