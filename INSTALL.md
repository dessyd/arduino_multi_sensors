# Installation Guide

## Required Libraries

### Core Libraries (Required)

Install via Arduino Library Manager:

1. **WiFiNINA** - WiFi support for MKR WiFi 1010
2. **Arduino_MKRENV** - MKR ENV Shield sensors
3. **RTCZero** - Real-time clock
4. **SD** - SD card support (usually pre-installed)
5. **SPI** - SPI communication (usually pre-installed)

### Optional Libraries

#### Grove Air Quality Sensor (Optional)

The Air Quality sensor support is **optional**. The code will compile and run without it.

**Option 1: Compile WITHOUT Air Quality sensor**

Edit `SensorManager.h` and comment out line 10:
```cpp
// #define USE_AIR_QUALITY_SENSOR  // Commented out = no Air Quality sensor
```

**Option 2: Install Air Quality sensor library**

If you have the Grove Air Quality sensor hardware:

1. Download the library from:
   https://github.com/Seeed-Studio/Grove_Air_quality_Sensor

2. Install manually:
   - Download ZIP from GitHub
   - Arduino IDE → Sketch → Include Library → Add .ZIP Library
   - Select the downloaded ZIP file

3. Verify installation:
   - Arduino IDE → Sketch → Include Library
   - Look for "Grove Air quality Sensor"

## Hardware Setup

### Required Hardware
- Arduino MKR WiFi 1010
- MKR ENV Shield

### Optional Hardware
- Grove Air Quality Sensor (connect to A0)
- SD Card (FAT32 formatted)

## Configuration

1. Create/edit `arduino_secrets.h`:
```cpp
#define SECRET_SSID "your-wifi-network-name"
#define SECRET_PASS "your-wifi-password"
#define SECRET_SPLUNK_SERVER "splunk.example.com"
```

2. Compile and upload to MKR WiFi 1010

3. Open Serial Monitor at 9600 baud to see status

## Troubleshooting

### "Air_Quality_Sensor.h: No such file or directory"

**Solution**: Comment out `#define USE_AIR_QUALITY_SENSOR` in `SensorManager.h` (line 10)

The system will work fine without the Air Quality sensor - only MKR ENV shield sensors will be used.

### "WiFiNINA.h: No such file or directory"

**Solution**: Install WiFiNINA library via Arduino Library Manager

### "Arduino_MKRENV.h: No such file or directory"

**Solution**: Install Arduino_MKRENV library via Arduino Library Manager

### Compilation warnings about unused variables

This is normal and safe to ignore. The code uses conditional compilation.

## Quick Start

Minimal configuration (MKR ENV shield only, no Air Quality sensor):

1. Install: WiFiNINA, Arduino_MKRENV, RTCZero
2. Edit `SensorManager.h`: Comment out line 10 (`#define USE_AIR_QUALITY_SENSOR`)
3. Configure `arduino_secrets.h`
4. Compile and upload
5. Done!

## Testing

See [TESTING.md](TESTING.md) for validation procedures.

Test UDP reception:
```bash
python3 test_udp_receiver.py --port 8125
```
