#include "Utils.h"
#include <WiFiNINA.h>

void getBoardID(char* boardID) {
  byte mac[MAC_LENGTH];
  WiFi.macAddress(mac);
  arrayToHexString(mac, MAC_LENGTH, boardID);
}

void arrayToHexString(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[len - i - 1] >> 4) & 0x0F;
    byte nib2 = (array[len - i - 1] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  buffer[len * 2] = '\0';
}

void formatStatsdMessage(char* buffer, size_t bufferSize,
                         const char* measureName, float value,
                         const char* boardID) {
  // Format: "sensor.<measure_name>:<value>|g|#board_id:<id>,board_type:mkr1010,sensor_type:mkr_env"
  snprintf(buffer, bufferSize,
           "sensor.%s:%.2f|g|#board_id:%s,board_type:mkr1010,sensor_type:mkr_env",
           measureName, value, boardID);
}
