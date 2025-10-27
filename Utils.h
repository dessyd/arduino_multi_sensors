#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "Config.h"

// MAC address conversion utilities
void getBoardID(char* boardID);
void arrayToHexString(byte array[], unsigned int len, char buffer[]);

// StatD message formatting
void formatStatsdMessage(char* buffer, size_t bufferSize,
                         const char* measureName, float value,
                         const char* boardID);

#endif // UTILS_H
