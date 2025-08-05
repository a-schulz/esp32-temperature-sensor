```bash
# On Raspberry Pi
chmod +x setup.sh
./setup.sh

# Upload ESP32 code
pio run -e ds18b20-mqtt -t upload --upload-port=/dev/ttyACM0
```