# ESP32 MQTT Client for SmartSafe (ESP-IDF C)

This is an ESP-IDF C implementation for connecting the ESP32 to the MQTT broker.

## Prerequisites

1. Install ESP-IDF (v5.0 or later)
   ```bash
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh esp32
   . ./export.sh
   ```

2. Install required ESP-IDF components:
   - `mqtt` (included in ESP-IDF)
   - `json` (cJSON, included in ESP-IDF)

## Configuration

1. Edit `main/mqtt_client.c` and update WiFi credentials:
   ```c
   #define WIFI_SSID      "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
   ```

## Building and Flashing

```bash
cd esp32_mqtt
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

(Replace `/dev/ttyUSB0` with your ESP32's serial port)

## Features

- Connects to WiFi
- Connects to MQTT broker at `alderaan.software-engineering.ie:1883`
- Subscribes to `smartsafe/safe/command` to receive commands
- Publishes telemetry to `smartsafe/safe/telemetry` every 10 seconds
- Parses JSON commands: `unlock`, `lock`, `set_code`

## MQTT Topics

- **Telemetry (publish):** `smartsafe/safe/telemetry`
- **Commands (subscribe):** `smartsafe/safe/command`

## Testing

From the server, send a command:
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"unlock"}'
```

The ESP32 will receive it and print to serial monitor.

