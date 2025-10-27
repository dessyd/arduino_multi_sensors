#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <SD.h>
#include <RTCZero.h>
#include "Config.h"

class DataLogger {
public:
  DataLogger(int chipSelect);

  // Initialization
  bool begin();
  bool initRTC();

  // Logging functions
  void logMessage(LogSeverity severity, const char* message);
  void logMeasurement(const char* measureName, float value);

  // RTC management
  bool updateTimeFromNTP();
  RTCZero& getRTC() { return m_rtc; }

  // SD Card status
  bool isSDCardAvailable() const { return m_sdInitialized; }

private:
  int m_chipSelect;
  bool m_sdInitialized;
  RTCZero m_rtc;

  // Helper functions
  void writeToFile(const char* entry, const char* fileExtension);
  void buildTimestamp(char* buffer, size_t bufferSize);
  void buildFilename(char* buffer, const char* extension);
};

#endif // DATA_LOGGER_H
