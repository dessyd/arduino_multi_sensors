#include "SensorManager.h"

SensorManager::SensorManager()
  : m_mkrEnvAvailable(false)
  , m_airQualityAvailable(false)
#ifdef USE_AIR_QUALITY_SENSOR
  , m_airQualitySensor(AIR_QUALITY_PIN)
#endif
{
}

int SensorManager::detectAndInitialize() {
  int sensorCount = 0;

  // Initialize MKR ENV shield
  m_mkrEnvAvailable = ENV.begin();

  if (m_mkrEnvAvailable) {
    if (Serial) Serial.println(F("Found MKR ENV shield"));
    sensorCount++;
  } else {
    if (Serial) Serial.println(F("WARNING: Failed to initialize MKR ENV shield"));
  }

  // Initialize Air Quality sensor (requires 20s warmup)
#ifdef USE_AIR_QUALITY_SENSOR
  if (Serial) Serial.println(F("Waiting for Air Quality sensor to initialize (20s)..."));
  delay(AIR_QUALITY_INIT_TIME);

  m_airQualityAvailable = m_airQualitySensor.init();

  if (m_airQualityAvailable) {
    if (Serial) Serial.println(F("Found Air Quality sensor"));
    sensorCount++;
  } else {
    if (Serial) Serial.println(F("WARNING: Failed to initialize Air Quality sensor"));
  }
#else
  m_airQualityAvailable = false;
  if (Serial) Serial.println(F("INFO: Air Quality sensor support not compiled (library not available)"));
#endif

  return sensorCount;
}

EnvironmentalData SensorManager::readEnvironmental() {
  EnvironmentalData data = {0};

  if (!m_mkrEnvAvailable) {
    return data;
  }

  // Read all environmental values
  data.temperature = ENV.readTemperature(CELSIUS) - DELTA_TEMP;
  data.humidity = ENV.readHumidity();
  data.pressure = ENV.readPressure(MILLIBAR);
  data.illuminance = ENV.readIlluminance(LUX);
  data.uva = ENV.readUVA();
  data.uvb = ENV.readUVB();
  data.uvIndex = ENV.readUVIndex();

  return data;
}

AirQualityData SensorManager::readAirQuality() {
  AirQualityData data = {0};
  data.isValid = false;

  if (!m_airQualityAvailable) {
    return data;
  }

#ifdef USE_AIR_QUALITY_SENSOR
  data.slope = m_airQualitySensor.slope();
  data.value = m_airQualitySensor.getValue();
  data.isValid = true;
#endif

  return data;
}

const char* SensorManager::getAirQualityDescription(int slope) {
#ifdef USE_AIR_QUALITY_SENSOR
  switch (slope) {
    case AirQualitySensor::FORCE_SIGNAL:
      return "High pollution! Force signal active.";
    case AirQualitySensor::HIGH_POLLUTION:
      return "High pollution!";
    case AirQualitySensor::LOW_POLLUTION:
      return "Low pollution!";
    case AirQualitySensor::FRESH_AIR:
      return "Fresh air.";
    default:
      return "Unknown air quality status";
  }
#else
  return "Air quality sensor not available";
#endif
}

LogSeverity SensorManager::getAirQualitySeverity(int slope) {
#ifdef USE_AIR_QUALITY_SENSOR
  switch (slope) {
    case AirQualitySensor::FORCE_SIGNAL:
      return SEV_CRITICAL;
    case AirQualitySensor::HIGH_POLLUTION:
      return SEV_HIGH;
    case AirQualitySensor::LOW_POLLUTION:
      return SEV_LOW;
    case AirQualitySensor::FRESH_AIR:
      return SEV_INFO;
    default:
      return SEV_MEDIUM;
  }
#else
  (void)slope; // Suppress unused parameter warning
  return SEV_INFO;
#endif
}
