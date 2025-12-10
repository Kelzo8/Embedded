# Quick Start Guide

## Step 1: Configure WiFi Credentials

Edit the config file:
```bash
cd esp32_mqtt
nano main/config.h
```

Or open `esp32_mqtt/main/config.h` in your editor and update:
- `WIFI_SSID`: Your WiFi network name
- `WIFI_PASSWORD`: Your WiFi password

The MQTT broker and device ID are already configured correctly.

## Step 2: Build and Flash

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

(On Mac, find your port with: `ls /dev/cu.*` - it might be `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART`)

## Step 3: Verify ESP32 Connection

In the serial monitor, you should see:
- ✅ "WiFi connected, IP: ..."
- ✅ "MQTT connected"
- ✅ "Subscribed to smartsafe/safe/command"
- ✅ "Published telemetry: ..." (every 10 seconds)

## Step 4: Test from Server

### Terminal 1: Watch Telemetry (what ESP32 sends)

SSH into server:
```bash
ssh jamesk@alderaan.software-engineering.ie
mosquitto_sub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/telemetry" -v
```

You should see telemetry messages every 10 seconds!

### Terminal 2: Send Commands (ESP32 receives)

```bash
ssh jamesk@alderaan.software-engineering.ie
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"unlock"}'
```

**Check ESP32 serial monitor** - you should see:
```
MQTT data received:
  Topic: smartsafe/safe/command
  Data: {"command":"unlock"}
Command: UNLOCK
```

## Test All Commands

**Unlock:**
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"unlock"}'
```

**Lock:**
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"lock"}'
```

**Set Code:**
```bash
mosquitto_pub -h alderaan.software-engineering.ie -p 1883 -t "smartsafe/safe/command" \
  -m '{"command":"set_code","code":"5621"}'
```

## Troubleshooting

**Can't find serial port:**
```bash
ls /dev/cu.*
```

**WiFi not connecting:**
- Check credentials in `main/config.h`
- Check WiFi signal strength
- Look for error messages in serial monitor

**MQTT not connecting:**
- Verify ESP32 can reach internet (check WiFi IP)
- Check serial monitor for MQTT errors

