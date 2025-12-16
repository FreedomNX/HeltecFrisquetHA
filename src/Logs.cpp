#include "Logs.h"

Logs logs;  // définition unique de l’instance globale

void debug(String message) {
    logs.addLog("DEBUG", message);
};
void info(String message) {
    logs.addLog("INFO", message);
}
void error(String message) {
    logs.addLog("ERROR", message);
}
void warning(String message) {
    logs.addLog("WARNING", message);
}
void debug(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    logs.addLog("DEBUG", buffer);
}


String byteArrayToHexString(const byte* byteArray, int length) {
  String result = "";
  for (int i = 0; i < length; i++)
  {
    char hex[3];
    sprintf(hex, "%02X", byteArray[i]);
    if(i > 0) {
        result += " ";
    }
    result += hex;
  }
  return result;
}

void logRadio(bool rx, const byte* payload, size_t length) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[%s][%d] %s", rx ? "RX" : "TX", length, byteArrayToHexString(payload, length).c_str());
    logs.addLog("RADIO", buffer);
}

void info(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    logs.addLog("INFO", buffer);
}

void error(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    logs.addLog("ERROR", buffer);
}

void warning(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    logs.addLog("WARNING", buffer);
}