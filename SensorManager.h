#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino_MKRENV.h>
#include "Air_Quality_Sensor.h"
#include "Config.h"

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
  AirQualitySensor m_airQualitySensor;
};

#endif // SENSOR_MANAGER_H
