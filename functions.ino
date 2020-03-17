// Format UDP message according to statsd protocol
void sendMeasure(char* m_name, float m_value) {
  char m_entry[126];
  int m_value_int, m_value_dec;

  // "sensor.<measure_name>:<measure_value>|g|#<dimension_name>:<dimension_value>,..."
  Udp.beginPacket(splunk_ip, STATSD_PORT_NUMBER);
  Udp.print("sensor.");
  Udp.print(m_name);
  Udp.print(":");
  Udp.print(m_value);
  Udp.print("|g|#board_id:");
  Udp.print(board_id);
  Udp.print(",board_type:mkr1010,sensor_type:mkr_env");
  Udp.endPacket();

  // Separate integer and decimal part of a float
  m_value_int = m_value;
  m_value_dec = (m_value - m_value_int) * 100.0;

  // Format measure for file storage: timestamp measure_name=measure_value
  sprintf(m_entry, "20%02d-%02d-%02d %02d:%02d:%02d.000,%s=%d.%02d", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), m_name, m_value_int, m_value_dec);
  //  Write to disk
  write2file( m_entry, "csv");
}


// Logging utility
#define LOG_LENGTH 126

void send2log( int severity, char* message ) {

  char log[LOG_LENGTH];

  // Build log message including timestamp
  sprintf(log, "20%02d-%02d-%02d %02d:%02d:%02d.000 %d %s", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), severity, message);

  if (Serial) Serial.println( log );
  write2file(log, "log");

}

// Write to SD Card
#define FILENAME_LENGTH 8+1+3+1
// 8 chars name + . + 3 char extension + EOL

void write2file( char * entry, char * file_extension ) {
  File sdcard_file;
  char filename[FILENAME_LENGTH];

  // Build filename as being today's date:followed by specified extension YYYYMMDD.file_extension
  sprintf(filename, "20%02d%02d%02d.%s", rtc.getYear(), rtc.getMonth(), rtc.getDay(), file_extension);

  if (SD.begin(chipSelect)) { // SD card available
    sdcard_file = SD.open(filename, FILE_WRITE);
    if (sdcard_file) {
      sdcard_file.println( entry );
      sdcard_file.close();
    }
  }
}

void configNetwork() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    send2log( SEV_CRITICAL, "WiFi shield not present!" );
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    send2log(SEV_INFO, "Please upgrade the WiFi firmware");
  }

  // attempt to connect to Wifi network:
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:

  do {
    WiFi.begin(ssid, pass);
    if (Serial) Serial.print(".");
    // Wait for connection
    delay ( 500 );
  }
  while ( WiFi.status() != WL_CONNECTED);

  if (Serial) Serial.println();
  send2log(SEV_INFO, "Wifi connected"); // Timestamp is empty at this stage

  // you're connected now, so print out the status:
  if (Serial) printWiFiStatus();

}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Sets internal RTC from external NTP source
void configRtc() {

  unsigned long epoch;
  int numberOfTries = 0, maxTries = 10;

  rtc.begin();

  do {
    epoch = WiFi.getTime();
    if (Serial) Serial.print ( "." );
    numberOfTries++;
    delay(1000);
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (Serial) Serial.println();

  if (numberOfTries == maxTries) {
    send2log(SEV_MEDIUM, "NTP unreachable!!");
  }

  rtc.setEpoch(epoch);

}

void getBoardID(char board_id[])
{
  // Gets board MAC address
  byte mac[ MAC_LENGTH ];           // Holds board MAC address
  WiFi.macAddress(mac);
  array_to_string(mac, 6, board_id);
}

// Convert MAC address to its corresponding HEX string
void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[len - i - 1] >> 4) & 0x0F;
    byte nib2 = (array[len - i - 1] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}

/*
  Returns the number of detected environmental sensors
  and sets the coresponding boolean to true.

  This will be used by the measurement routine to poll the righ sensor
*/
int detectedSensors() {

  int sensor_count = 0;

  // Check if ENV board is present
  mkr_env = ENV.begin();

  if (mkr_env) {
    send2log(SEV_INFO, "Found MKR ENV shield.");
    sensor_count++;
  }
  else {
    send2log(SEV_LOW, "Failed to initialize MKR ENV shield!");
  }

  send2log(SEV_INFO, "Waiting AirQuality sensor to init...");
  delay(20000);
  air_quality = air_quality_sensor.init();

  if (air_quality) {
    send2log(SEV_INFO, "Found Air Quality sensor.");
    sensor_count++;
  }
  else {
    send2log(SEV_LOW, "Failed to initialize Air Qaulity sensor!");
  }
  return sensor_count;
}

//
// Read MKR ENV shield sensors values
//
void readMKR(boolean enabled) {
  if (enabled) {
    // read all the sensor values
    temperature = ENV.readTemperature(CELSIUS) - DELTA_TEMP;
    humidity    = ENV.readHumidity();
    pressure    = ENV.readPressure(MILLIBAR);
    illuminance = ENV.readIlluminance(LUX);
    uva         = ENV.readUVA();
    uvb         = ENV.readUVB();
    uvIndex     = ENV.readUVIndex();

    sendMeasure("temperature", temperature);
    sendMeasure("humidity", humidity);
    sendMeasure("pressure", pressure);
    sendMeasure("illuminance", illuminance);
    sendMeasure("uva", uva);
    sendMeasure("uvb", uvb);
    sendMeasure("uvIndex", uvIndex);
  }

}

//
// Read Grove Air Quality sensor value
//
void readAirQuality(boolean enabled) {
  int quality = 0;

  if (enabled) {
    quality = air_quality_sensor.slope();

    sendMeasure("AirQuality", air_quality_sensor.getValue());

    if (quality == AirQualitySensor::FORCE_SIGNAL) {
      send2log(SEV_CRITICAL, "High pollution! Force signal active.");
    }
    else if (quality == AirQualitySensor::HIGH_POLLUTION) {
      send2log(SEV_HIGH, "High pollution!");
    }
    else if (quality == AirQualitySensor::LOW_POLLUTION) {
      send2log(SEV_LOW, "Low pollution!");
    }
    else if (quality == AirQualitySensor::FRESH_AIR) {
      send2log(SEV_INFO, "Fresh air.");
    }
  }
}

//
// Read Grove Air VOC sensor value
//
void readAirVOC(boolean enabled) {
  if (enabled) {

  }

}