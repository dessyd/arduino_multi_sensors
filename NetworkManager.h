#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "Config.h"

class NetworkManager {
public:
  NetworkManager(const char* ssid, const char* pass, const char* serverHostname);

  // Initialization
  bool begin();

  // Connection management
  bool isWiFiConnected();
  bool reconnectWiFi();

  // UDP management
  bool beginUDP();
  bool isUDPReady();

  // Server resolution
  bool resolveServer();
  IPAddress getServerIP() const { return m_serverIP; }

  // Data sending
  bool sendUDPPacket(const char* data);

  // Utility
  void printStatus();
  const char* getFirmwareVersion();

private:
  const char* m_ssid;
  const char* m_pass;
  const char* m_serverHostname;

  WiFiUDP m_udp;
  IPAddress m_serverIP;
  bool m_udpInitialized;
  bool m_serverResolved;

  bool checkWiFiModule();
};

#endif // NETWORK_MANAGER_H
