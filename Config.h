#ifndef CONFIG_H
#define CONFIG_H

// Timing constants
const unsigned long SENSOR_READ_INTERVAL = 60000;  // 60 seconds between readings
const unsigned long SERIAL_TIMEOUT = 60000;         // 1 minute wait for Serial
const unsigned long AIR_QUALITY_INIT_TIME = 20000;  // 20 seconds for AirQuality sensor init
const unsigned long NETWORK_RETRY_DELAY = 500;      // 500ms between network retries
const unsigned long NTP_RETRY_DELAY = 1000;         // 1 second between NTP retries

// Network configuration
const int MAX_NTP_RETRIES = 10;
const int STATSD_PORT_NUMBER = 8125;

// Hardware configuration
const int SD_CHIP_SELECT = 4;
const int MAC_LENGTH = 6;
const int AIR_QUALITY_PIN = A0;

// Buffer sizes
const size_t STATSD_BUFFER_SIZE = 126;
const size_t LOG_BUFFER_SIZE = 126;
const size_t FILENAME_LENGTH = 13;  // 8 chars + . + 3 ext + null

// Sensor calibration
const float DELTA_TEMP = 1.6;  // Temperature correction for MKR board self-heating

// Log severity levels
enum LogSeverity {
  SEV_INFO = 1,
  SEV_LOW = 2,
  SEV_MEDIUM = 3,
  SEV_HIGH = 4,
  SEV_CRITICAL = 5
};

// Board identification
extern char g_boardID[2 * MAC_LENGTH + 1];

#endif // CONFIG_H
