# ESP32 DHT11 Temperature Sensor Project

This project utilizes an ESP32 microcontroller along with a DHT11 temperature and humidity sensor to read and display temperature and humidity data.

## Project Structure

- **src/**: Contains the main application code.
- **include/**: Header files for the project.
- **lib/**: Libraries used in the project.
- **platformio.ini**: Configuration file for PlatformIO.
- **README.md**: Documentation for the project.

## Hardware Requirements

- ESP32 development board
- DHT11 temperature and humidity sensor
- Jumper wires
- Breadboard (optional)
- USB cable for connecting ESP32 to computer

## Software Requirements

- PlatformIO IDE
- DHT sensor library by Adafruit

## Installation

1. Install PlatformIO IDE.
2. Clone this repository or download the project files.
3. Open the project in PlatformIO.
4. Install the required libraries specified in `platformio.ini`.

## Wiring

Connect the DHT11 sensor to the ESP32 as follows:

- DHT11 VCC → ESP32 3.3V
- DHT11 GND → ESP32 GND
- DHT11 DATA → ESP32 GPIO pin 4 (can be changed in code)

## Usage

1. Connect your ESP32 to your computer.
2. Select the correct board and port in PlatformIO.
3. Upload the code.
4. Open the Serial Monitor to view temperature and humidity readings (115200 baud but platformio should detect automatically).

## Troubleshooting

- If no readings appear, check your wiring connections.
- Ensure the DHT11 sensor is functioning properly.
- Verify that the correct GPIO pin is specified in the code.

## Setup on NixOS
https://nixos.wiki/wiki/Platformio
