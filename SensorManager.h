#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino_MKRENV.h>
#include "Config.h"

// Optional Air Quality Sensor support
// Install library from: https://github.com/Seeed-Studio/Grove_Air_quality_Sensor
// Or comment out the next line to compile without Air Quality sensor
#define USE_AIR_QUALITY_SENSOR

#ifdef USE_AIR_QUALITY_SENSOR
  #include "Air_Quality_Sensor.h"
#endif

struct EnvironmentalData {
  float temperature;
  float humidity;
  float pressure;
  float illuminance;
  float uva;
  float uvb;
  float uvIndex;
};

struct AirQualityData {
  float value;
  int slope;
  bool isValid;
};

class SensorManager {
public:
  SensorManager();

  // Initialization
  int detectAndInitialize();

  // Sensor availability
  bool hasMKREnv() const { return m_mkrEnvAvailable; }
  bool hasAirQuality() const { return m_airQualityAvailable; }

  // Reading functions
  EnvironmentalData readEnvironmental();
  AirQualityData readAirQuality();

  // Status reporting
  const char* getAirQualityDescription(int slope);
  LogSeverity getAirQualitySeverity(int slope);

private:
  bool m_mkrEnvAvailable;
  bool m_airQualityAvailable;

#ifdef USE_AIR_QUALITY_SENSOR
  AirQualitySensor m_airQualitySensor;
#endif
};

#endif // SENSOR_MANAGER_H
