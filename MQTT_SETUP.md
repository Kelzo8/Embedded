# SmartSafe MQTT Setup

## MQTT Topics

The MQTT broker is already running on `alderaan.software-engineering.ie` port `1883`.

### Telemetry Topic (ESP32 → Cloud)
**Topic:** `smartsafe/<device_id>/telemetry`

**Example:** `smartsafe/safe/telemetry`

**Payload format (JSON):**
```json
{
  "timestamp": 1700000000,
  "motion": true,
  "alarm": false,
  "state": "locked",
  "code_ok": false
}
```

### Command Topic (Cloud → ESP32)
**Topic:** `smartsafe/<device_id>/command`

**Example:** `smartsafe/safe/command`

**Payload formats (JSON):**
```json
{ "command": "unlock" }
```
```json
{ "command": "set_code", "code": "5621" }
```
```json
{ "command": "lock" }
```

## Quick Test with mosquitto tools

### Subscribe to telemetry (see what ESP32 sends):
```bash
mosquitto_sub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/telemetry" -v
```

### Publish test telemetry:
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/telemetry" \
  -m '{"timestamp":1700000000,"motion":true,"alarm":false,"state":"locked","code_ok":false}'
```

### Subscribe to commands (see what cloud sends to ESP32):
```bash
mosquitto_sub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" -v
```

### Send a command:
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"unlock"}'
```

## ESP32 Code Example (ESP-IDF C)

See `esp32_mqtt/` directory for a complete ESP-IDF C implementation showing how to:
- Connect to WiFi
- Connect to MQTT broker
- Subscribe to `smartsafe/safe/command` to receive commands
- Publish telemetry to `smartsafe/safe/telemetry`

**Required:**
- ESP-IDF v5.0 or later
- WiFi credentials configured in `main/mqtt_client.c`

**Key parts:**
1. `wifi_init()` - Initializes and connects to WiFi
2. `mqtt_event_handler()` - Handles MQTT events and parses incoming commands
3. `publish_telemetry()` - Creates JSON and publishes telemetry data
4. `telemetry_task()` - Periodically publishes telemetry every 10 seconds

**Build and flash:**
```bash
cd esp32_mqtt
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```


