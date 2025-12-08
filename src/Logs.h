#pragma once

#include <heltec.h>
#include <map>
#include <vector>
#include <TimeLib.h>

class Logs {
public:
  explicit Logs(size_t maxLogSize = 500) : _maxLogSize(maxLogSize) {}

  struct Line {
    Line(String level, String message) : level(level), message(message), time(now()) {}
    
    String toString() {
      char buffer[256];
      char date[20];
      strftime (buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&time));
      snprintf(buffer, sizeof(buffer), "[%s][%s] %s", level.c_str(), date, message.c_str());
      return String(buffer);
    }

    String level;
    String message;
    time_t time;
  };

  void clear() { _logs.clear(); }

  // Ajouter un log avec un niveau
  void addLog(const String& level, const String& message) {
    if (_logs.size() >= _maxLogSize) {
      // Si la capacité est dépassée, on supprime la première entrée
      _logs.erase(_logs.begin());
    }
    Line line = Line(level, message);
    _logs.push_back(line);
    Serial.println(line.toString());
  }

  // Ajouter un log formaté
  void addLogf(const String& level, const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    addLog(level, String(buffer));
  }

  // Récupérer les logs pour un niveau donné
  String getLogsByLevel(const String& level) {
    String result = "";
    for (Line& line : _logs) {
      if(line.level.equals(level)) {
        result += line.toString() + "\n";
      }
    }
    return result;
  }

  // Récupérer tous les logs
  String getAllLogs() {
    String result = "";
    for (Line& line : _logs) {
      if(line.level != "RADIO") {
        result += line.toString() + "\n";
      }
    }
    return result;
  }

  size_t getLogCount(const String& level) {
    return _logs.size();
  }

private:
  size_t _maxLogSize;
  std::vector<Line> _logs;  // Associe le niveau de log à une liste de messages
};

extern Logs logs;

void debug(String message);
void debug(const char* fmt, ...);
void logRadio(bool rx, const byte* payload, size_t length);
void info(String message);
void info(const char* fmt, ...);
void error(String message);
void error(const char* fmt, ...);
void warning(String message);
void warrning(const char* fmt, ...);
