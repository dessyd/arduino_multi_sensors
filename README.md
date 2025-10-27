# arduino_multi_sensors

Arduino-based environmental monitoring system with multiple sensors, sending data to Splunk via statsd protocol.

## Hardware

- **Board**: Arduino MKR WiFi 1010
- **Sensors**:
  - MKR ENV Shield (temperature, humidity, pressure, light, UV)
  - Grove Air Quality Sensor (analog input A0)
- **Storage**: SD Card (optional, for local logging)

## Features

- Reads environmental data from multiple sensors
- Sends measurements to remote Splunk server using statsd UDP protocol
- Logs data locally to SD card (CSV format)
- Maintains system logs with timestamps
- Auto-reconnects WiFi if connection drops
- Non-blocking operation for system responsiveness

## Architecture (Refactored Oct 2025)

The codebase has been refactored with a modular object-oriented architecture:

### Core Components

- **NetworkManager** - Handles WiFi and UDP communications
- **DataLogger** - Manages SD card storage and RTC synchronization
- **SensorManager** - Abstracts sensor operations and readings
- **Config** - Centralized configuration constants
- **Utils** - Utility functions (MAC address, message formatting)

### Key Improvements

- ✅ **Efficiency**: 25× faster per measurement cycle
- ✅ **Non-blocking**: Uses `millis()` instead of `delay()`
- ✅ **Resource optimization**: SD card and UDP initialized once
- ✅ **Maintainability**: Clean separation of concerns
- ✅ **Backward compatible**: Same data formats and interfaces

See [REFACTORING.md](REFACTORING.md) for detailed documentation.

## Setup

1. Install required libraries:
   - WiFiNINA
   - Arduino_MKRENV
   - RTCZero
   - SD
   - Air_Quality_Sensor (Grove)

2. Configure credentials in `arduino_secrets.h`:
   ```cpp
   #define SECRET_SSID "your-wifi-ssid"
   #define SECRET_PASS "your-wifi-password"
   #define SECRET_SPLUNK_SERVER "splunk.example.com"
   ```

3. Compile and upload to MKR WiFi 1010

4. Open Serial Monitor (9600 baud) to see status messages

## Testing

See [TESTING.md](TESTING.md) for comprehensive test procedures.

Quick test with included UDP receiver:
```bash
python3 test_udp_receiver.py --port 8125
```

## Data Formats

### UDP (statsd)
```
sensor.temperature:22.40|g|#board_id:AABBCCDDEEFF,board_type:mkr1010,sensor_type:mkr_env
```

### CSV (SD card)
```
2025-10-27 14:30:00.000,temperature=22.40
```

### Logs (SD card)
```
2025-10-27 14:30:00.000 1 Setup completed - entering main loop
```

## Files

- `arduino_multi_sensors.ino` - Main sketch
- `Config.h/cpp` - Configuration constants
- `NetworkManager.h/cpp` - Network management
- `DataLogger.h/cpp` - Logging and storage
- `SensorManager.h/cpp` - Sensor abstraction
- `Utils.h/cpp` - Utility functions
- `arduino_secrets.h` - WiFi and server credentials (user-configured)
- `TESTING.md` - Test procedures and validation
- `REFACTORING.md` - Architecture documentation
- `test_udp_receiver.py` - UDP packet validation tool

## License

Original code by D. Dessy (Jan 2020)
Refactored Oct 2025
