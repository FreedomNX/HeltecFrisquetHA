#pragma once

#include <Arduino.h>
#include <TimeLib.h>
#include <time.h>
#include <stdarg.h>

// ---------------------------
// Configuration
// ---------------------------
#ifndef LOGS_MAX_LINES
  #define LOGS_MAX_LINES 500
#endif

#ifndef LOGS_LEVEL_LEN
  #define LOGS_LEVEL_LEN 8   // ex: "WARNING" + \0
#endif

#ifndef LOGS_MSG_LEN
  #define LOGS_MSG_LEN 160   // ajuste selon tes besoins
#endif

// ---------------------------
// Modèle de log (sans String)
// ---------------------------
struct LogLine {
  time_t time;
  char level[LOGS_LEVEL_LEN];
  char message[LOGS_MSG_LEN];

  void toString(char* out, size_t outSize) const {
    char date[20];
    struct tm tm_info;
    localtime_r(&time, &tm_info);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &tm_info);

    snprintf(out, outSize, "[%s][%s] %s", level, date, message);
  }
};

class Logs {
public:
  Logs() = default;

  void clear() {
    _count = 0;
    _head = 0;
  }

  // Ajout simple
  void addLog(const char* level, const char* message) {
    LogLine& line = pushLine();
    line.time = now();
    safeCopy(line.level, sizeof(line.level), level);
    safeCopy(line.message, sizeof(line.message), message);

    // Impression immédiate (sans String)
    char buffer[256];
    line.toString(buffer, sizeof(buffer));
    Serial.println(buffer);
  }

  // Ajout formaté
  void addLogf(const char* level, const char* fmt, ...) {
    char msg[LOGS_MSG_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    addLog(level, msg);
  }

  // Compte total (ou par niveau)
  size_t getLogCount(const char* level = nullptr) const {
    if (!level || level[0] == '\0') return _count;

    size_t c = 0;
    forEach([&](const LogLine& l) {
      if (strcmp(l.level, level) == 0) c++;
    }, /*levelFilter*/ level, /*excludeRadio*/ false);

    return c;
  }

  // Parcours des lignes (le plus fiable, 0 allocation)
  // - callback appelée dans l'ordre chronologique (ancien -> récent)
  // - levelFilter : nullptr pour tout
  // - excludeRadio : true pour ignorer "RADIO"
  template<typename Fn>
  void forEach(Fn&& fn, const char* levelFilter = nullptr, bool excludeRadio = true) const {
    const size_t n = _count;
    if (n == 0) return;

    // index du plus ancien
    const size_t start = oldestIndex();

    for (size_t i = 0; i < n; i++) {
      const LogLine& line = _lines[(start + i) % LOGS_MAX_LINES];

      if (excludeRadio && strcmp(line.level, "RADIO") == 0) continue;
      if (levelFilter && levelFilter[0] != '\0' && strcmp(line.level, levelFilter) != 0) continue;

      fn(line);
    }
  }

  // Récupérer les N dernières lignes dans un tableau fourni (sans vector)
  // - outCapacity = taille du tableau out[]
  // - retourne le nombre copié
  size_t getLastLines(LogLine* out, size_t outCapacity,
                      size_t limit = 1000,
                      const char* levelFilter = nullptr,
                      bool excludeRadio = true) const {
    if (!out || outCapacity == 0 || limit == 0 || _count == 0) return 0;

    const size_t n = _count;
    const size_t maxTake = min(min(limit, outCapacity), n);

    size_t copied = 0;

    // On part du plus récent et on remonte, puis on remet en ordre chrono dans out
    // index du plus récent :
    size_t idx = newestIndex();

    while (copied < maxTake) {
      const LogLine& line = _lines[idx];

      bool ok = true;
      if (excludeRadio && strcmp(line.level, "RADIO") == 0) ok = false;
      if (ok && levelFilter && levelFilter[0] != '\0' && strcmp(line.level, levelFilter) != 0) ok = false;

      if (ok) {
        out[maxTake - 1 - copied] = line; // écrit à l'envers → au final c'est chronologique
        copied++;
      }

      if (idx == 0) idx = LOGS_MAX_LINES - 1;
      else idx--;

      // sécurité : si on a bouclé sur tous les éléments mais filtrage trop strict
      if (copied < maxTake && copied + 1 > n) break;
    }

    // Si filtrage strict, on peut avoir moins que maxTake
    // Dans ce cas on compacte en début de tableau
    if (copied < maxTake) {
      const size_t shift = maxTake - copied;
      for (size_t i = 0; i < copied; i++) out[i] = out[i + shift];
    }

    return copied;
  }

private:
  LogLine _lines[LOGS_MAX_LINES];
  size_t _head = 0;   // prochain emplacement d'écriture
  size_t _count = 0;  // nb de logs valides (<= LOGS_MAX_LINES)

  LogLine& pushLine() {
    LogLine& line = _lines[_head];
    _head = (_head + 1) % LOGS_MAX_LINES;
    if (_count < LOGS_MAX_LINES) _count++;
    return line;
  }

  size_t oldestIndex() const {
    if (_count < LOGS_MAX_LINES) return 0;
    return _head; // quand plein, _head pointe sur le plus ancien (prochaine écriture l'écrase)
  }

  size_t newestIndex() const {
    if (_count == 0) return 0;
    // _head pointe sur prochaine écriture, donc le plus récent est juste avant
    return (_head == 0) ? (LOGS_MAX_LINES - 1) : (_head - 1);
  }

  static void safeCopy(char* dst, size_t dstSize, const char* src) {
    if (!dst || dstSize == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
  }
};

// Instance globale comme tu faisais
extern Logs logs;

// Helpers (sans String)
/*inline void debug(const char* msg)   { logs.addLog("DEBUG", msg); }
inline void info(const char* msg)    { logs.addLog("INFO", msg); }
inline void warning(const char* msg) { logs.addLog("WARN", msg); }
inline void error(const char* msg)   { logs.addLog("ERROR", msg); }*/

inline void debug(const char* fmt, ...) {
  va_list args; va_start(args, fmt);
  char tmp[LOGS_MSG_LEN];
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  logs.addLog("DEBUG", tmp);
}
inline void info(const char* fmt, ...) {
  va_list args; va_start(args, fmt);
  char tmp[LOGS_MSG_LEN];
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  logs.addLog("INFO", tmp);
}
inline void warning(const char* fmt, ...) {
  va_list args; va_start(args, fmt);
  char tmp[LOGS_MSG_LEN];
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  logs.addLog("WARN", tmp);
}
inline void error(const char* fmt, ...) {
  va_list args; va_start(args, fmt);
  char tmp[LOGS_MSG_LEN];
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  va_end(args);
  logs.addLog("ERROR", tmp);
}

// Exemple radio: tu peux garder ton byteArrayToHexString mais SANS String
inline void logRadio(bool rx, const byte* payload, size_t length) {
  char hex[LOGS_MSG_LEN];
  size_t pos = 0;

  pos += snprintf(hex + pos, sizeof(hex) - pos, "%s ", rx ? "RX" : "TX");
  for (size_t i = 0; i < length && pos + 3 < sizeof(hex); i++) {
    pos += snprintf(hex + pos, sizeof(hex) - pos, "%02X%s", payload[i], (i + 1 < length) ? " " : "");
  }

  logs.addLog("RADIO", hex);
}
