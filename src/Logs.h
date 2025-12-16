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
    
    String toString () const {
      char buffer[256];
      char date[20];
      strftime (date, 20, "%Y-%m-%d %H:%M:%S", localtime(&time));
      snprintf(buffer, sizeof(buffer), "[%s][%s] %s", level.c_str(), date, message.c_str());
      return String(buffer);
    }

    String level;
    String message;
    time_t time;
  };

  void clear() { 
    while (_busy) { delay(1); }
    _busy = true;
    _logs.clear();
    _busy = false;
  }

  // Ajouter un log avec un niveau
  void addLog(const String& level, const String& message) {
    while(_busy) { delay(1); }

    _busy = true;
    if (_logs.size() >= _maxLogSize) {
      // Si la capacité est dépassée, on supprime la première entrée
      _logs.erase(_logs.begin());
    }
    Line line = Line(level, message);
    _logs.push_back(line);

    _busy = false;
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

  String getLogsByLevel(const String& level) {
  // Attendre qu'aucune écriture/clonage ne soit en cours
  while (_busy) { delay(1); }
  _busy = true;

  String result;
  for (Line& line : _logs) {
    if (line.level.equals(level)) {
      result += line.toString() + "\n";
    }
  }
  _busy = false;
  return result;
}

String getAllLogs() {
  while (_busy) { delay(1); }
  _busy = true;

  String result;
  for (Line& line : _logs) {
    if (!line.level.equals("RADIO")) {
      result += line.toString() + "\n";
    }
  }
  _busy = false;
  return result;
}

size_t getLogCount(const String& level) {
  while (_busy) { delay(1); }
  _busy = true;

  if (level.length() == 0) {
    return _logs.size();
  }

  size_t count = 0;
  for (const Line& line : _logs) {
    if (line.level.equals(level)) {
      ++count;
    }
  }
  _busy = false;
  return count;
}

  void getLines(std::vector<Line>& out, size_t limit = 1000, const String& level = "") {
    out.clear();
    if (limit == 0) return;

    // On attend qu'aucune écriture ne soit en cours
    while (_busy) { delay(1); }
    _busy = true;

    if (_logs.empty()) return;

    bool filterByLevel = (level.length() > 0);

    // On part de la fin (logs les plus récentes)
    for (int i = (int)_logs.size() - 1; i >= 0 && out.size() < limit; --i) {
      const Line line = _logs[i];

      if (filterByLevel) {
        if (!line.level.equals(level)) {
          continue;
        }
      } else if(line.level.equals("RADIO")) {
        continue;
      }

      out.push_back(line);
    }
    _busy = false;

    // Là on a les logs dans l'ordre "plus récent → plus ancien",
    // on les remet dans l'ordre chronologique "plus ancien → plus récent"
    std::reverse(out.begin(), out.end());
  }

private:
  size_t _maxLogSize;
  std::vector<Line> _logs;  // Associe le niveau de log à une liste de messages
  volatile bool _busy = false;
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
