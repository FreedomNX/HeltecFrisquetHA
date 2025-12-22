#include "Logs.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <time.h>

Logs logs;  // définition unique de l’instance globale

void Logs::Line::set(const char* levelIn, const char* messageIn, time_t t) {
  Logs::copyTruncate(level, sizeof(level), levelIn);
  Logs::copyTruncate(message, sizeof(message), messageIn);
  time = t;
}

bool Logs::Line::levelEquals(const char* other) const {
  if (!other || other[0] == '\0') {
    return false;
  }
  return strncmp(level, other, sizeof(level)) == 0;
}

void Logs::Line::format(char* out, size_t outSize) const {
  if (!out || outSize == 0) {
    return;
  }
  char date[20];
  const tm* local = localtime(&time);
  if (local) {
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", local);
  } else {
    Logs::copyTruncate(date, sizeof(date), "1970-01-01 00:00:00");
  }
  snprintf(out, outSize, "[%s][%s] %s", level, date, message);
}

void Logs::copyTruncate(char* dest, size_t destSize, const char* src) {
  if (!dest || destSize == 0) {
    return;
  }
  if (!src) {
    dest[0] = '\0';
    return;
  }
  strncpy(dest, src, destSize - 1);
  dest[destSize - 1] = '\0';
}

void Logs::clear() {
  //BusyGuard guard(*this);
  _count = 0;
  _head = 0;
}

void Logs::addLog(const char* level, const char* message) {
  if (_capacity == 0) {
    return;
  }
  char formatted[kMaxFormattedLen];
  Line line;
  line.set(level ? level : "", message ? message : "", now());

  {
    //BusyGuard guard(*this);
    _entries[_head] = line;
    _head = (_head + 1) % _capacity;
    if (_count < _capacity) {
      ++_count;
    }
  }

  line.format(formatted, sizeof(formatted));
  Serial.println(formatted);
}

void Logs::addLogf(const char* level, const char* fmt, ...) {
  char buffer[kMaxMessageLen];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  addLog(level, buffer);
}

size_t Logs::getLogCount(const char* level) {
  if (_capacity == 0) {
    return 0;
  }
  //BusyGuard guard(*this);
  if (!level || level[0] == '\0') {
    return _count;
  }
  size_t count = 0;
  for (size_t i = 0; i < _count; ++i) {
    size_t idx = (_head + _capacity - _count + i) % _capacity;
    if (_entries[idx].levelEquals(level)) {
      ++count;
    }
  }
  return count;
}

size_t Logs::getLines(Line* out, size_t outCapacity, size_t limit, const char* level) {
  if (!out || outCapacity == 0 || limit == 0 || _capacity == 0) {
    return 0;
  }

  //BusyGuard guard(*this);
  if (_count == 0) {
    return 0;
  }

  bool filterByLevel = level && level[0] != '\0';
  size_t maxOut = limit < outCapacity ? limit : outCapacity;
  size_t outCount = 0;

  for (size_t i = 0; i < _count && outCount < maxOut; ++i) {
    size_t idx = (_head + _capacity - 1 - i) % _capacity;
    const Line& line = _entries[idx];

    if (filterByLevel) {
      if (!line.levelEquals(level)) {
        continue;
      }
    } else if (line.levelEquals("RADIO")) {
      continue;
    }

    out[outCount++] = line;
  }

  for (size_t i = 0; i < outCount / 2; ++i) {
    Line tmp = out[i];
    out[i] = out[outCount - 1 - i];
    out[outCount - 1 - i] = tmp;
  }

  return outCount;
}

void debug(const String& message) {
  logs.addLog("DEBUG", message.c_str());
}

void info(const String& message) {
  logs.addLog("INFO", message.c_str());
}

void error(const String& message) {
  logs.addLog("ERROR", message.c_str());
}

void warning(const String& message) {
  logs.addLog("WARNING", message.c_str());
}

void debug(const char* fmt, ...) {
  char buffer[Logs::kMaxMessageLen];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  logs.addLog("DEBUG", buffer);
}

static void formatHex(char* out, size_t outSize, const byte* payload, size_t length) {
  if (!out || outSize == 0) {
    return;
  }
  if (!payload || length == 0) {
    out[0] = '\0';
    return;
  }
  size_t pos = 0;
  out[0] = '\0';
  for (size_t i = 0; i < length && pos + 3 < outSize; ++i) {
    int written = snprintf(out + pos, outSize - pos, "%s%02X", i > 0 ? " " : "", payload[i]);
    if (written <= 0) {
      break;
    }
    pos += static_cast<size_t>(written);
  }
}

void logRadio(bool rx, const byte* payload, size_t length) {
  char hex[Logs::kMaxMessageLen];
  formatHex(hex, sizeof(hex), payload, length);
  char buffer[Logs::kMaxMessageLen];
  snprintf(buffer, sizeof(buffer), "[%s][%d] %s", rx ? "RX" : "TX", static_cast<int>(length), hex);
  logs.addLog("RADIO", buffer);
}

void info(const char* fmt, ...) {
  char buffer[Logs::kMaxMessageLen];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  logs.addLog("INFO", buffer);
}

void error(const char* fmt, ...) {
  char buffer[Logs::kMaxMessageLen];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  logs.addLog("ERROR", buffer);
}

void warning(const char* fmt, ...) {
  char buffer[Logs::kMaxMessageLen];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  logs.addLog("WARNING", buffer);
}

void warrning(const char* fmt, ...) {
  char buffer[Logs::kMaxMessageLen];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  logs.addLog("WARNING", buffer);
}
