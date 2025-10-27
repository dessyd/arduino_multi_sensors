#include "NetworkManager.h"

NetworkManager::NetworkManager(const char* ssid, const char* pass, const char* serverHostname)
  : m_ssid(ssid)
  , m_pass(pass)
  , m_serverHostname(serverHostname)
  , m_udpInitialized(false)
  , m_serverResolved(false) {
}

bool NetworkManager::begin() {
  // Check for WiFi module
  if (!checkWiFiModule()) {
    return false;
  }

  // Connect to WiFi
  if (!reconnectWiFi()) {
    return false;
  }

  // Initialize UDP once
  if (!beginUDP()) {
    return false;
  }

  // Resolve server hostname once
  if (!resolveServer()) {
    return false;
  }

  return true;
}

bool NetworkManager::checkWiFiModule() {
  if (WiFi.status() == WL_NO_MODULE) {
    if (Serial) Serial.println(F("ERROR: WiFi shield not present!"));
    return false;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    if (Serial) Serial.println(F("WARNING: Please upgrade WiFi firmware"));
  }

  return true;
}

bool NetworkManager::reconnectWiFi() {
  // Already connected?
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  if (Serial) Serial.print(F("Connecting to WiFi"));

  // Attempt connection
  do {
    WiFi.begin(m_ssid, m_pass);
    if (Serial) Serial.print(".");
    delay(NETWORK_RETRY_DELAY);
  } while (WiFi.status() != WL_CONNECTED);

  if (Serial) {
    Serial.println();
    Serial.println(F("WiFi connected!"));
  }

  return true;
}

bool NetworkManager::isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::beginUDP() {
  // Only initialize once
  if (m_udpInitialized) {
    return true;
  }

  int status = m_udp.begin(STATSD_PORT_NUMBER);
  if (status == 0) {
    if (Serial) Serial.println(F("ERROR: No UDP socket available!"));
    return false;
  }

  m_udpInitialized = true;
  return true;
}

bool NetworkManager::isUDPReady() {
  return m_udpInitialized;
}

bool NetworkManager::resolveServer() {
  // Use cached IP if already resolved
  if (m_serverResolved) {
    return true;
  }

  int status = WiFi.hostByName(m_serverHostname, m_serverIP);
  if (status == 0) {
    if (Serial) {
      Serial.print(F("ERROR: Cannot resolve server: "));
      Serial.println(m_serverHostname);
    }
    return false;
  }

  m_serverResolved = true;
  if (Serial) {
    Serial.print(F("Server IP: "));
    Serial.println(m_serverIP);
  }

  return true;
}

bool NetworkManager::sendUDPPacket(const char* data) {
  if (!m_udpInitialized || !m_serverResolved) {
    return false;
  }

  m_udp.beginPacket(m_serverIP, STATSD_PORT_NUMBER);
  m_udp.print(data);
  return m_udp.endPacket() == 1;
}

void NetworkManager::printStatus() {
  if (!Serial) return;

  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

const char* NetworkManager::getFirmwareVersion() {
  static String fv = WiFi.firmwareVersion();
  return fv.c_str();
}
