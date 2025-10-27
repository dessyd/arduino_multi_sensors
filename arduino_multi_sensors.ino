/*
  Sends environmental data from multiple sensors to a Splunk instance
  using the statsd protocol.

  Refactored version with modular architecture and optimized resource usage.

  D. Dessy
  Jan 2020 - Refactored Oct 2025
*/

#include "arduino_secrets.h"
#include "Config.h"
#include "NetworkManager.h"
#include "DataLogger.h"
#include "SensorManager.h"
#include "Utils.h"

// Secrets from arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char splunk_server[] = SECRET_SPLUNK_SERVER;

// Global managers
NetworkManager* networkManager = nullptr;
DataLogger* dataLogger = nullptr;
SensorManager* sensorManager = nullptr;

// Timing for non-blocking operation
unsigned long lastSensorRead = 0;

// Reusable buffer for messages
char messageBuffer[STATSD_BUFFER_SIZE];

void setup() {
  Serial.begin(9600);

  // Wait for Serial connection (non-blocking with timeout)
  unsigned long serialStartTime = millis();
  while (!Serial && (millis() - serialStartTime < SERIAL_TIMEOUT)) {
    // Wait for serial or timeout
  }

  if (Serial) {
    Serial.println(F("========================================"));
    Serial.println(F("Arduino Multi-Sensor System - Refactored"));
    Serial.println(F("========================================"));
  }

  // Initialize managers
  networkManager = new NetworkManager(ssid, pass, splunk_server);
  dataLogger = new DataLogger(SD_CHIP_SELECT);
  sensorManager = new SensorManager();

  // Get board unique ID
  getBoardID(g_boardID);
  if (Serial) {
    Serial.print(F("Board ID: "));
    Serial.println(g_boardID);
  }

  // Initialize network
  if (!networkManager->begin()) {
    dataLogger->logMessage(SEV_CRITICAL, "Network initialization failed!");
    while (true); // Halt on critical error
  }

  dataLogger->logMessage(SEV_INFO, "Network initialized");
  networkManager->printStatus();

  // Initialize RTC
  if (!dataLogger->initRTC()) {
    dataLogger->logMessage(SEV_MEDIUM, "RTC initialization failed - timestamps may be incorrect");
  } else {
    dataLogger->logMessage(SEV_INFO, "RTC synchronized");
  }

  // Initialize SD card (non-critical)
  dataLogger->begin();

  // Detect and initialize sensors
  int sensorCount = sensorManager->detectAndInitialize();
  if (sensorCount == 0) {
    dataLogger->logMessage(SEV_CRITICAL, "No sensors detected, aborting!");
    while (true); // Halt on critical error
  }

  snprintf(messageBuffer, sizeof(messageBuffer), "Detected %d sensor(s)", sensorCount);
  dataLogger->logMessage(SEV_INFO, messageBuffer);
  dataLogger->logMessage(SEV_INFO, "Setup completed - entering main loop");

  if (Serial) {
    Serial.println(F("========================================"));
    Serial.println();
  }
}

void loop() {
  unsigned long currentTime = millis();

  // Check if it's time to read sensors (non-blocking)
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentTime;

    // Ensure WiFi is connected
    if (!networkManager->isWiFiConnected()) {
      dataLogger->logMessage(SEV_LOW, "WiFi disconnected - attempting reconnection");
      if (!networkManager->reconnectWiFi()) {
        dataLogger->logMessage(SEV_HIGH, "WiFi reconnection failed");
        return; // Try again next cycle
      }
      dataLogger->logMessage(SEV_INFO, "WiFi reconnected");
    }

    // Verify network is ready
    if (!networkManager->isUDPReady()) {
      dataLogger->logMessage(SEV_HIGH, "UDP not ready");
      return; // Try again next cycle
    }

    // Read and send environmental data
    if (sensorManager->hasMKREnv()) {
      readAndSendEnvironmentalData();
    }

    // Read and send air quality data
    if (sensorManager->hasAirQuality()) {
      readAndSendAirQualityData();
    }

    if (Serial) {
      Serial.println(F("--- Sensor read cycle completed ---"));
      Serial.println();
    }
  }

  // Other non-blocking tasks could be added here
}

void readAndSendEnvironmentalData() {
  EnvironmentalData envData = sensorManager->readEnvironmental();

  // Send each measurement via statsd and log to SD
  sendMeasure("temperature", envData.temperature);
  sendMeasure("humidity", envData.humidity);
  sendMeasure("pressure", envData.pressure);
  sendMeasure("illuminance", envData.illuminance);
  sendMeasure("uva", envData.uva);
  sendMeasure("uvb", envData.uvb);
  sendMeasure("uvIndex", envData.uvIndex);

  if (Serial) {
    Serial.println(F("Environmental data sent"));
  }
}

void readAndSendAirQualityData() {
  AirQualityData aqData = sensorManager->readAirQuality();

  if (!aqData.isValid) {
    dataLogger->logMessage(SEV_LOW, "Air quality data invalid");
    return;
  }

  // Send measurement
  sendMeasure("AirQuality", aqData.value);

  // Log air quality status
  LogSeverity severity = sensorManager->getAirQualitySeverity(aqData.slope);
  const char* description = sensorManager->getAirQualityDescription(aqData.slope);
  dataLogger->logMessage(severity, description);

  if (Serial) {
    Serial.print(F("Air quality: "));
    Serial.println(description);
  }
}

void sendMeasure(const char* measureName, float value) {
  // Format statsd message
  formatStatsdMessage(messageBuffer, sizeof(messageBuffer),
                      measureName, value, g_boardID);

  // Send via UDP
  if (!networkManager->sendUDPPacket(messageBuffer)) {
    snprintf(messageBuffer, sizeof(messageBuffer),
             "Failed to send %s via UDP", measureName);
    dataLogger->logMessage(SEV_LOW, messageBuffer);
  }

  // Log measurement to SD card
  dataLogger->logMeasurement(measureName, value);
}
