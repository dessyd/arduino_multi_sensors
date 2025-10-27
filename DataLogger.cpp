#include "DataLogger.h"
#include <WiFiNINA.h>

DataLogger::DataLogger(int chipSelect)
  : m_chipSelect(chipSelect)
  , m_sdInitialized(false) {
}

bool DataLogger::begin() {
  // Initialize SD card once
  m_sdInitialized = SD.begin(m_chipSelect);

  if (!m_sdInitialized) {
    if (Serial) Serial.println(F("WARNING: SD Card not available"));
  } else {
    if (Serial) Serial.println(F("SD Card initialized"));
  }

  return m_sdInitialized;
}

bool DataLogger::initRTC() {
  m_rtc.begin();
  return updateTimeFromNTP();
}

bool DataLogger::updateTimeFromNTP() {
  unsigned long epoch;
  int numberOfTries = 0;

  if (Serial) Serial.print(F("Syncing time from NTP"));

  do {
    epoch = WiFi.getTime();
    if (Serial) Serial.print(".");
    numberOfTries++;
    delay(NTP_RETRY_DELAY);
  } while ((epoch == 0) && (numberOfTries < MAX_NTP_RETRIES));

  if (Serial) Serial.println();

  if (numberOfTries >= MAX_NTP_RETRIES) {
    if (Serial) Serial.println(F("WARNING: NTP unreachable!"));
    return false;
  }

  m_rtc.setEpoch(epoch);
  if (Serial) {
    Serial.print(F("Time synchronized: "));
    Serial.print(m_rtc.getYear() + 2000);
    Serial.print(F("-"));
    Serial.print(m_rtc.getMonth());
    Serial.print(F("-"));
    Serial.println(m_rtc.getDay());
  }

  return true;
}

void DataLogger::buildTimestamp(char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "20%02d-%02d-%02d %02d:%02d:%02d.000",
           m_rtc.getYear(), m_rtc.getMonth(), m_rtc.getDay(),
           m_rtc.getHours(), m_rtc.getMinutes(), m_rtc.getSeconds());
}

void DataLogger::buildFilename(char* buffer, const char* extension) {
  snprintf(buffer, FILENAME_LENGTH, "20%02d%02d%02d.%s",
           m_rtc.getYear(), m_rtc.getMonth(), m_rtc.getDay(), extension);
}

void DataLogger::writeToFile(const char* entry, const char* fileExtension) {
  if (!m_sdInitialized) {
    return;  // SD card not available
  }

  char filename[FILENAME_LENGTH];
  buildFilename(filename, fileExtension);

  File sdFile = SD.open(filename, FILE_WRITE);
  if (sdFile) {
    sdFile.println(entry);
    sdFile.close();
  } else {
    if (Serial) {
      Serial.print(F("ERROR: Cannot open file: "));
      Serial.println(filename);
    }
  }
}

void DataLogger::logMessage(LogSeverity severity, const char* message) {
  char logBuffer[LOG_BUFFER_SIZE];
  char timestamp[32];

  buildTimestamp(timestamp, sizeof(timestamp));

  // Build log message with timestamp
  snprintf(logBuffer, sizeof(logBuffer), "%s %d %s", timestamp, severity, message);

  // Print to Serial
  if (Serial) Serial.println(logBuffer);

  // Write to SD card
  writeToFile(logBuffer, "log");
}

void DataLogger::logMeasurement(const char* measureName, float value) {
  char measureBuffer[LOG_BUFFER_SIZE];
  char timestamp[32];

  buildTimestamp(timestamp, sizeof(timestamp));

  // Format: timestamp,measure_name=value
  snprintf(measureBuffer, sizeof(measureBuffer), "%s,%s=%.2f",
           timestamp, measureName, value);

  // Write to CSV file
  writeToFile(measureBuffer, "csv");
}
