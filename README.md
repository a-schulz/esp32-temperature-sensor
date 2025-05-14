# ESP32 DHT11 Temperature Sensor

This project uses an ESP32 microcontroller and a DHT11 temperature/humidity sensor to read temperature data and output it to the serial monitor.

## Hardware Requirements

- ESP32 development board
- DHT11 temperature and humidity sensor
- Jumper wires
- Breadboard (optional)
- USB cable for connecting ESP32 to computer

## Wiring

Connect the DHT11 sensor to the ESP32 as follows:

- DHT11 VCC → ESP32 3.3V
- DHT11 GND → ESP32 GND
- DHT11 DATA → ESP32 GPIO pin 4 (can be changed in code)

## Software Requirements

- Arduino IDE
- DHT sensor library by Adafruit

## Installation

1. Install the Arduino IDE
2. Add ESP32 board support to Arduino IDE
   - Go to File → Preferences
   - Add `https://dl.espressif.com/dl/package_esp32_index.json` to Additional Boards Manager URLs
   - Go to Tools → Board → Boards Manager and install "ESP32"
3. Install required libraries:
   - Go to Sketch → Include Library → Manage Libraries
   - Search for and install "DHT sensor library" by Adafruit
   - Also install "Adafruit Unified Sensor" library

## Usage

1. Connect your ESP32 to your computer
2. Select the correct board and port in Arduino IDE
3. Upload the sketch
4. Open the Serial Monitor at 115200 baud to see temperature readings

## Troubleshooting

- If no readings appear, check your wiring connections
- Ensure the DHT11 sensor is not damaged
- Verify that the correct GPIO pin is specified in the code

## Last Updated

May 14, 2025