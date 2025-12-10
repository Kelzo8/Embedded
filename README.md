# Embedded SmartSafe Project

## MQTT Cloud Backend

The MQTT broker (mosquitto) is running on `alderaan.software-engineering.ie` port `1883`.

See [MQTT_SETUP.md](MQTT_SETUP.md) for:
- Topic structure and examples
- Quick test commands using mosquitto tools

### Quick Start

**Test telemetry subscription:**
```bash
mosquitto_sub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/telemetry" -v
```

**Send a command:**
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"unlock"}'
```

See `esp32_mqtt/QUICK_START.md` for ESP32 setup and testing instructions.