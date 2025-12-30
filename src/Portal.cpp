#include "Portal.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <stdarg.h>
#include <cstring>
#include "Frisquet/NetworkID.h" 

// Déclaration du logger global défini dans Logs.cpp
extern Logs logs;

static constexpr size_t kMaxQueryLines = 120;
static Logs::Line s_logLines[kMaxQueryLines];

// Helpers NetworkID <-> String "AA:BB:CC:DD"
static String networkIdToStr(const NetworkID& id) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X",
           id.bytes[0], id.bytes[1], id.bytes[2], id.bytes[3]);
  return String(buf);
}

static bool parseUint16Hex(const String& input, uint16_t& out) {
  String s = input;
  s.trim();
  if (s.length() == 0) {
    return false;
  }

  char* end = nullptr;
  unsigned long value = strtoul(s.c_str(), &end, 16);
  if (!end || *end != '\0' || value > 0xFFFFUL) {
    return false;
  }
  out = static_cast<uint16_t>(value);
  return true;
}

static bool parseNetworkIdFromString(const String& s, NetworkID& out) {
  String hex = s;
  hex.trim();
  hex.replace(":", "");
  hex.replace("-", "");
  hex.toUpperCase();

  if (hex.length() != 8) return false;

  uint8_t b[4];
  for (int i = 0; i < 4; ++i) {
    String sub = hex.substring(i * 2, i * 2 + 2);
    char* end = nullptr;
    long v = strtol(sub.c_str(), &end, 16);
    if (!end || *end != '\0' || v < 0 || v > 255) {
      return false;
    }
    b[i] = (uint8_t)v;
  }

  out = NetworkID(b[0], b[1], b[2], b[3]);
  return true;
}

static String jsonEscape(const String& s) {
    String out;
    out.reserve(s.length() + 10);

    for (uint16_t i = 0; i < s.length(); i++) {
        char c = s[i];

        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if ((uint8_t)c < 0x20) {
                    // Caractère de contrôle → tout en \u00XX
                    char buf[7];
                    sprintf(buf, "\\u%04X", (uint8_t)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

// Helper pour parser un bool depuis une string
static bool parseBoolArg(const String& s, bool defaultVal) {
  String v = s;
  v.trim();
  v.toLowerCase();
  if (v == "1" || v == "true" || v == "on"  || v == "yes" || v == "oui")  return true;
  if (v == "0" || v == "false"|| v == "off" || v == "no"  || v == "non") return false;
  return defaultVal;
}

Portal::Portal(FrisquetManager& frisquetManager, uint16_t port)
: _srv(port), _frisquetManager(frisquetManager) {}

void Portal::begin(bool startApFallbackIfNoWifi) {
  if (startApFallbackIfNoWifi && !WiFi.isConnected()) {
    startAp();
  }

  _srv.on("/", HTTP_GET, [this]{ handleIndex(); });
  _srv.on("/api/ping", HTTP_GET, [this]{ handlePing(); });
  _srv.on("/api/config", HTTP_GET, [this]{ handleGetConfig(); });
  _srv.on("/api/config", HTTP_POST, [this]{ handlePostConfig(); });
  _srv.on("/api/reboot", HTTP_POST, [this]{ handleReboot(); });
  _srv.on("/api/logs", HTTP_GET, [this]{ handleGetLogs(); });
  _srv.on("/api/logs/clear", HTTP_POST, [this]{ handleClearLogs(); });
  _srv.on("/logs", HTTP_GET, [this]{ handleLogsPage(); });
  _srv.on("/api/status", HTTP_GET, [this]{ handleStatus(); });
  _srv.on("/logs-radio", HTTP_GET, [this]{ handleRadioLogsPage(); });
  _srv.on("/api/memory", HTTP_GET, [this]{ handleMemoryRead(); });
  _srv.on("/memory", HTTP_GET, [this]{ handleMemoryPage(); });
  _srv.on("/api/radio/send", HTTP_POST, [this]{ handleSendRadio(); });
  _srv.on("/api/connect/pair", HTTP_POST, [this]{ handlePairConnect(); });
  _srv.on("/api/sonde-ext/pair", HTTP_POST, [this]{ handlePairSondeExt(); });
  _srv.on("/api/satellite/z1/pair", HTTP_POST, [this]{ handlePairSatelliteZ1(); });
  _srv.on("/api/satellite/z2/pair", HTTP_POST, [this]{ handlePairSatelliteZ2(); });
  _srv.on("/api/satellite/z3/pair", HTTP_POST, [this]{ handlePairSatelliteZ3(); });


  _srv.onNotFound([this](){
    _srv.send(404, "text/plain; charset=utf-8", "404 Non trouvé");
  });

  _srv.begin();
  info("[PORTAIL] Serveur HTTP démarré");
}

void Portal::loop() {
  _srv.handleClient();
}

void Portal::handleClearLogs() {
  logs.clear();
  _srv.send(200, "text/plain; charset=utf-8", "OK");
}

// -------------------- API --------------------

void Portal::handleIndex() {
  _srv.send(200, "text/html; charset=utf-8", html());
}

void Portal::handlePing() {
  _srv.send(200, "application/json", "{\"ok\":true}");
}

void Portal::handleGetConfig() {
  auto& w = _frisquetManager.config().getWiFiOptions();
  auto& m = _frisquetManager.config().getMQTTOptions();

  String json = "{";
  json += "\"wifiHostname\":\"" + jsonEscape(String(w.hostname)) + "\",";
  json += "\"wifiSsid\":\"" + jsonEscape(String(w.ssid)) + "\",";
  json += "\"wifiPass\":\"" + jsonEscape(String(w.password)) + "\",";
  json += "\"mqttHost\":\"" + jsonEscape(String(m.host)) + "\",";
  json += "\"mqttPort\":" + jsonEscape(String(m.port)) + ",";
  json += "\"mqttUser\":\"" + jsonEscape(String(m.username)) + "\",";
  json += "\"mqttPass\":\"" + jsonEscape(String(m.password)) + "\",";
  json += "\"mqttClientId\":\"" + jsonEscape(String(m.clientId)) + "\",";
  json += "\"mqttBaseTopic\":\"" + jsonEscape(String(m.baseTopic)) + "\",";

  // --- Frisquet ---
  const NetworkID& nid = _frisquetManager.config().getNetworkID(); // adapte le nom si besoin
  json += "\"networkID\":\"" + jsonEscape(networkIdToStr(nid)) + "\",";

  json += "\"useConnect\":" +
          String(_frisquetManager.config().useConnect() ? "true" : "false") + ",";
  json += "\"useSondeExt\":" +
          String(_frisquetManager.config().useSondeExterieure() ? "true" : "false") + ",";
  json += "\"useDS18B20\":" +
          String(_frisquetManager.config().useDS18B20() ? "true" : "false") + ",";
   
  // Satellites physiques
  json += "\"useSatelliteZ1\":" +
          String(_frisquetManager.config().useSatelliteZ1() ? "true" : "false") + ",";
  json += "\"useSatelliteZ2\":" +
          String(_frisquetManager.config().useSatelliteZ2() ? "true" : "false") + ",";
  json += "\"useSatelliteZ3\":" +
          String(_frisquetManager.config().useSatelliteZ3() ? "true" : "false") + ",";

  // Satellites virtuels (remplacement)
  json += "\"useSatelliteVirtualZ1\":" +
          String(_frisquetManager.config().useSatelliteVirtualZ1() ? "true" : "false") + ",";
  json += "\"useSatelliteVirtualZ2\":" +
          String(_frisquetManager.config().useSatelliteVirtualZ2() ? "true" : "false") + ",";
  json += "\"useSatelliteVirtualZ3\":" +
          String(_frisquetManager.config().useSatelliteVirtualZ3() ? "true" : "false");

  json += "}";
  _srv.send(200, "application/json; charset=utf-8", json);
}

void Portal::handlePostConfig() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json", "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  auto& w = _frisquetManager.config().getWiFiOptions();
  auto& m = _frisquetManager.config().getMQTTOptions();

  // Champs WiFi
  if (_srv.hasArg("wifiHostname")) w.hostname = _srv.arg("wifiHostname");
  if (_srv.hasArg("wifiSsid"))     w.ssid     = _srv.arg("wifiSsid");
  if (_srv.hasArg("wifiPass"))     w.password = _srv.arg("wifiPass");

  // Champs MQTT
  if (_srv.hasArg("mqttHost"))     m.host     = _srv.arg("mqttHost");
  if (_srv.hasArg("mqttPort"))     m.port     = (uint16_t)_srv.arg("mqttPort").toInt();
  if (_srv.hasArg("mqttUser"))     m.username = _srv.arg("mqttUser");
  if (_srv.hasArg("mqttPass"))     m.password = _srv.arg("mqttPass");
  if (_srv.hasArg("mqttClientId")) m.clientId = _srv.arg("mqttClientId");
  if (_srv.hasArg("mqttBaseTopic"))m.baseTopic= _srv.arg("mqttBaseTopic");

  // --- Frisquet: NetworkID ---
  if (_srv.hasArg("networkID")) {
    String s = _srv.arg("networkID");
    NetworkID nid;
    if (parseNetworkIdFromString(s, nid)) {
      _frisquetManager.config().setNetworkID(nid);
      info("[PORTAIL] NetworkID mis à jour: %s", networkIdToStr(nid).c_str());
    } else {
      info("[PORTAIL] NetworkID invalide reçu: '%s'", s.c_str());
      // On laisse le NetworkID actuel si parsing ko
    }
  }

  // --- Frisquet: booleans ---
  if (_srv.hasArg("useConnect")) {
    bool cur = _frisquetManager.config().useConnect();
    bool v = parseBoolArg(_srv.arg("useConnect"), cur);
    _frisquetManager.config().useConnect(v);
  }
  if (_srv.hasArg("useSondeExt")) {
    bool cur = _frisquetManager.config().useSondeExterieure();
    bool v = parseBoolArg(_srv.arg("useSondeExt"), cur);
    _frisquetManager.config().useSondeExterieure(v);
  }
  if (_srv.hasArg("useDS18B20")) {
    bool cur = _frisquetManager.config().useDS18B20();
    bool v = parseBoolArg(_srv.arg("useDS18B20"), cur);
    _frisquetManager.config().useDS18B20(v);
  }

  if (_srv.hasArg("useSatelliteZ1")) {
    bool cur = _frisquetManager.config().useSatelliteZ1();
    bool v = parseBoolArg(_srv.arg("useSatelliteZ1"), cur);
    _frisquetManager.config().useSatelliteZ1(v);
  }
  if (_srv.hasArg("useSatelliteZ2")) {
    bool cur = _frisquetManager.config().useSatelliteZ2();
    bool v = parseBoolArg(_srv.arg("useSatelliteZ2"), cur);
    _frisquetManager.config().useSatelliteZ2(v);
  }
  if (_srv.hasArg("useSatelliteZ3")) {
    bool cur = _frisquetManager.config().useSatelliteZ3();
    bool v = parseBoolArg(_srv.arg("useSatelliteZ3"), cur);
    _frisquetManager.config().useSatelliteZ3(v);
  }

  // Satellites virtuels (remplacement du satellite physique)
  if (_srv.hasArg("useSatelliteVirtualZ1")) {
    bool cur = _frisquetManager.config().useSatelliteVirtualZ1();
    bool v = parseBoolArg(_srv.arg("useSatelliteVirtualZ1"), cur);
    _frisquetManager.config().useSatelliteVirtualZ1(v);
  }
  if (_srv.hasArg("useSatelliteVirtualZ2")) {
    bool cur = _frisquetManager.config().useSatelliteVirtualZ2();
    bool v = parseBoolArg(_srv.arg("useSatelliteVirtualZ2"), cur);
    _frisquetManager.config().useSatelliteVirtualZ2(v);
  }
  if (_srv.hasArg("useSatelliteVirtualZ3")) {
    bool cur = _frisquetManager.config().useSatelliteVirtualZ3();
    bool v = parseBoolArg(_srv.arg("useSatelliteVirtualZ3"), cur);
    _frisquetManager.config().useSatelliteVirtualZ3(v);
  }

  _frisquetManager.config().save();
  info("[PORTAIL] Configuration enregistrée, redémarrage programmé");

  _srv.send(200, "application/json; charset=utf-8", "{\"ok\":true,\"reboot\":true}");
  scheduleReboot(800);
}

void Portal::handleReboot() {
  _srv.send(200, "application/json", "{\"ok\":true}");
  scheduleReboot(200);
}

void Portal::handleGetLogs() {
  // ?limit=100 (par défaut)
  size_t limit = 100;
  if (_srv.hasArg("limit")) {
    int v = _srv.arg("limit").toInt();
    if (v > 0) {
      limit = (size_t)v;
    }
  }
  if (limit > kMaxQueryLines) {
    limit = kMaxQueryLines;
  }

  // ?level=INFO / DEBUG / ERROR / RADIO...
  String level;
  if (_srv.hasArg("level")) {
    level = _srv.arg("level");
  }

  size_t outCount = logs.getLines(s_logLines, kMaxQueryLines, limit,
                                  level.length() > 0 ? level.c_str() : nullptr);

  auto sendEscapedJson = [this](const char* text) {
    char buffer[128];
    size_t pos = 0;
    auto flush = [&]() {
      if (pos > 0) {
        buffer[pos] = '\0';
        _srv.sendContent(buffer);
        pos = 0;
      }
    };

    for (const char* p = text; p && *p; ++p) {
      const char* esc = nullptr;
      char unicode[7];
      switch (*p) {
        case '\"': esc = "\\\""; break;
        case '\\': esc = "\\\\"; break;
        case '\b': esc = "\\b"; break;
        case '\f': esc = "\\f"; break;
        case '\n': esc = "\\n"; break;
        case '\r': esc = "\\r"; break;
        case '\t': esc = "\\t"; break;
        default:
          if (static_cast<uint8_t>(*p) < 0x20) {
            snprintf(unicode, sizeof(unicode), "\\u%04X", static_cast<uint8_t>(*p));
            esc = unicode;
          }
          break;
      }

      if (!esc) {
        if (pos + 1 >= sizeof(buffer)) {
          flush();
        }
        buffer[pos++] = *p;
      } else {
        size_t len = strlen(esc);
        if (pos + len >= sizeof(buffer)) {
          flush();
        }
        memcpy(buffer + pos, esc, len);
        pos += len;
      }
    }
    flush();
  };

  _srv.setContentLength(CONTENT_LENGTH_UNKNOWN);
  _srv.send(200, "application/json; charset=utf-8", "");
  _srv.sendContent("[");
  bool first = true;
  for (size_t i = 0; i < outCount; ++i) {
    if (!first) {
      _srv.sendContent(",");
    }
    first = false;

    char formatted[Logs::kMaxFormattedLen];
    s_logLines[i].format(formatted, sizeof(formatted));
    _srv.sendContent("\"");
    sendEscapedJson(formatted);
    _srv.sendContent("\"");
  }
  _srv.sendContent("]");
}


void Portal::handleLogsPage() {
  _srv.send(200, "text/html; charset=utf-8", logsHtml());
}

void Portal::handleStatus() {
  bool sta = WiFi.isConnected();
  bool ap  = _apRunning;

  IPAddress ip = WiFi.localIP();
  String ssid = sta ? WiFi.SSID() : "";
  long rssi   = sta ? WiFi.RSSI() : 0;

  String json = "{";
  json += "\"apRunning\":"     + String(ap  ? "true" : "false") + ",";
  json += "\"staConnected\":"  + String(sta ? "true" : "false") + ",";
  json += "\"ssid\":\""        + ssid + "\",";
  json += "\"ip\":\""          + ip.toString() + "\",";
  json += "\"rssi\":"          + String(rssi);
  json += "}";
  _srv.send(200, "application/json; charset=utf-8", json);
}

void Portal::handleSendRadio() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_srv.hasArg("payload")) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Champ 'payload' manquant\"}");
    return;
  }

  String hex = _srv.arg("payload");
  hex.trim();

  if (!hex.length()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Payload vide\"}");
    return;
  }

  // Validation basique : hex + espaces
  for (size_t i = 0; i < hex.length(); ++i) {
    char c = hex[i];
    if (!( (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F') ||
           c == ' ')) {
      _srv.send(400, "application/json; charset=utf-8",
                "{\"ok\":false,\"err\":\"Payload non hexadécimal\"}");
      return;
    }
  }

  info("[PORTAIL] Demande d'envoi trame RADIO: %s", hex.c_str());

  byte payload[100];
  size_t payloadLength = 0;
  hexStringToBufferRaw(hex, payload, 100, payloadLength);

  logRadio(false, payload, payloadLength);
  //_frisquetManager.radio().interruptReceive = true;
  _frisquetManager.radio().transmit(payload, payloadLength);
  //_frisquetManager.radio().interruptReceive = false;

  bool ok = true;

  if (ok) {
    _srv.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
  } else {
    _srv.send(500, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Échec envoi radio\"}");
  }
}

void Portal::handleRadioLogsPage() {
  _srv.send(200, "text/html; charset=utf-8", logsRadioHtml());
}

void Portal::handleMemoryRead() {
  if (_srv.method() != HTTP_GET) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_frisquetManager.connect().estAssocie()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Connect non associé\"}");
    return;
  }

  uint16_t start = 0;
  uint16_t len = 16;

  if (_srv.hasArg("start")) {
    if (!parseUint16Hex(_srv.arg("start"), start)) {
      _srv.send(400, "application/json; charset=utf-8",
                "{\"ok\":false,\"err\":\"Paramètre start invalide\"}");
      return;
    }
  }

  if (_srv.hasArg("len")) {
    char* end = nullptr;
    unsigned long v = strtoul(_srv.arg("len").c_str(), &end, 10);
    if (!end || *end != '\0' || v == 0 || v > 256) {
      _srv.send(400, "application/json; charset=utf-8",
                "{\"ok\":false,\"err\":\"Paramètre len invalide (1..256)\"}");
      return;
    }
    len = static_cast<uint16_t>(v);
  }

  if (start > 0xFFFF - (len - 1)) {
    len = static_cast<uint16_t>(0xFFFF - start + 1);
  }

  uint16_t errorAddrs[256];
  uint16_t errorCodes[256];
  size_t errorCount = 0;

  auto sendUInt = [this](unsigned long v) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", v);
    _srv.sendContent(buf);
  };

  auto sendHex2 = [this](uint8_t v) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02X", v);
    _srv.sendContent(buf);
  };

  auto sendHex4 = [this](uint16_t v) {
    char buf[5];
    snprintf(buf, sizeof(buf), "%04X", v);
    _srv.sendContent(buf);
  };

  _srv.setContentLength(CONTENT_LENGTH_UNKNOWN);
  _srv.send(200, "application/json; charset=utf-8", "");
  _srv.sendContent("{\"ok\":true,\"start\":");
  sendUInt(start);
  _srv.sendContent(",\"startHex\":\"");
  sendHex4(start);
  _srv.sendContent("\",\"len\":");
  sendUInt(len);
  _srv.sendContent(",\"words\":[");

  for (uint16_t i = 0; i < len; ++i) {
    uint16_t addr = static_cast<uint16_t>(start + i);
    byte resp[32];
    size_t respLen = sizeof(resp);

    uint16_t err = _frisquetManager.radio().sendAsk(
        ID_CONNECT,
        ID_CHAUDIERE,
        _frisquetManager.connect().getIdAssociation(),
        0x01,
        0x01,
        addr,
        0x0002,
        resp,
        respLen,
      1);

    int value = -1;
    if (err == RADIOLIB_ERR_NONE) {
      size_t headerSize = sizeof(FrisquetRadio::RadioTrameHeader);
      if (respLen > headerSize) {
        uint8_t dataLen = resp[headerSize];
        if (dataLen >= 2 && respLen >= headerSize + 1 + dataLen) {
          uint8_t b1 = resp[headerSize + 1];
          uint8_t b2 = resp[headerSize + 2];
          value = (static_cast<int>(b1) << 8) | static_cast<int>(b2);
        } else if (respLen >= headerSize + 3) {
          uint8_t b1 = resp[respLen - 2];
          uint8_t b2 = resp[respLen - 1];
          value = (static_cast<int>(b1) << 8) | static_cast<int>(b2);
        }
      }
    }

    if (i > 0) {
      _srv.sendContent(",");
    }
    _srv.sendContent("\"");
    if (value >= 0) {
      sendHex4(static_cast<uint16_t>(value));
    } else {
      _srv.sendContent("??");
      if (errorCount < 256) {
        errorAddrs[errorCount] = addr;
        errorCodes[errorCount] = err == RADIOLIB_ERR_NONE ? 1 : err;
        ++errorCount;
      }
    }
    _srv.sendContent("\"");
  }

  _srv.sendContent("],\"errors\":[");
  for (size_t i = 0; i < errorCount; ++i) {
    if (i > 0) {
      _srv.sendContent(",");
    }
    _srv.sendContent("{\"addr\":\"");
    sendHex4(errorAddrs[i]);
    _srv.sendContent("\",\"err\":");
    sendUInt(errorCodes[i]);
    _srv.sendContent("}");
  }
  _srv.sendContent("]}");
}

void Portal::handleMemoryPage() {
  _srv.send(200, "text/html; charset=utf-8", memoryHtml());
}

void Portal::handlePairConnect() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_frisquetManager.config().useConnect()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Connect désactivé dans la configuration\"}");
    return;
  }

  info("[PORTAIL] Demande d'association du module Connect");

  bool ok = false;
  // Exemple : si tu as une méthode dédiée
  
  NetworkID networkId;
  uint8_t idAssociation;
  if(_frisquetManager.connect().associer(networkId, idAssociation)) {
      _frisquetManager.connect().setIdAssociation(idAssociation);
      _frisquetManager.radio().setNetworkID(networkId);
      _frisquetManager.config().setNetworkID(networkId);
      _frisquetManager.config().save();
      _frisquetManager.connect().saveConfig();
      info("[PORTAIL] Association réussie.");
      ok = true;
  } else {
    error("[PORTAIL] Échec de l'association.");
  }

  if (ok) {
    _srv.send(200, "application/json; charset=utf-8",
              "{\"ok\":true,\"msg\":\"Association Connect lancée\"}");
  } else {
    _srv.send(500, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Échec lancement association Connect\"}");
  }
}

void Portal::handlePairSondeExt() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_frisquetManager.config().useSondeExterieure()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Sonde extérieure désactivée dans la configuration\"}");
    return;
  }

  info("[PORTAIL] Demande d'association de la sonde extérieure");

  bool ok = false;
  
  NetworkID networkId;
  uint8_t idAssociation;
  if(_frisquetManager.sondeExterieure().associer(networkId, idAssociation)) {
      _frisquetManager.sondeExterieure().setIdAssociation(idAssociation);
      _frisquetManager.radio().setNetworkID(networkId);
      _frisquetManager.config().setNetworkID(networkId);
      _frisquetManager.config().save();
      _frisquetManager.sondeExterieure().saveConfig();
      info("[PORTAIL] Association réussie.");
      ok = true;
  } else {
    error("[PORTAIL] Échec de l'association.");
  }

  if (ok) {
    _srv.send(200, "application/json; charset=utf-8",
              "{\"ok\":true,\"msg\":\"Association sonde extérieure lancée\"}");
  } else {
    _srv.send(500, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Échec lancement association sonde extérieure\"}");
  }
}

void Portal::handlePairSatelliteZ1() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_frisquetManager.config().useSatelliteZ1()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Satellite Z1 désactivé dans la configuration\"}");
    return;
  }

  info("[PORTAIL] Demande d'association du Satellite Z1");

  bool ok = false;
  NetworkID networkId;
  uint8_t idAssociation;

  // ⚠️ À ADAPTER selon ton API réelle :
  //   satelliteZ1(), setIdAssociation(), saveConfig()...
  if (_frisquetManager.satelliteZ1().associer(networkId, idAssociation)) {
    _frisquetManager.satelliteZ1().setIdAssociation(idAssociation);
    _frisquetManager.radio().setNetworkID(networkId);
    _frisquetManager.config().setNetworkID(networkId);
    _frisquetManager.config().save();
    _frisquetManager.satelliteZ1().saveConfig();
    info("[PORTAIL] Association Satellite Z1 réussie.");
    ok = true;
  } else {
    error("[PORTAIL] Échec de l'association Satellite Z1.");
  }

  if (ok) {
    _srv.send(200, "application/json; charset=utf-8",
              "{\"ok\":true,\"msg\":\"Association Satellite Z1 lancée\"}");
  } else {
    _srv.send(500, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Échec lancement association Satellite Z1\"}");
  }
}

void Portal::handlePairSatelliteZ2() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_frisquetManager.config().useSatelliteZ2()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Satellite Z2 désactivé dans la configuration\"}");
    return;
  }

  info("[PORTAIL] Demande d'association du Satellite Z2");

  bool ok = false;
  NetworkID networkId;
  uint8_t idAssociation;

  // ⚠️ À ADAPTER selon ton API réelle :
  if (_frisquetManager.satelliteZ2().associer(networkId, idAssociation)) {
    _frisquetManager.satelliteZ2().setIdAssociation(idAssociation);
    _frisquetManager.radio().setNetworkID(networkId);
    _frisquetManager.config().setNetworkID(networkId);
    _frisquetManager.config().save();
    _frisquetManager.satelliteZ2().saveConfig();
    info("[PORTAIL] Association Satellite Z2 réussie.");
    ok = true;
  } else {
    error("[PORTAIL] Échec de l'association Satellite Z2.");
  }

  if (ok) {
    _srv.send(200, "application/json; charset=utf-8",
              "{\"ok\":true,\"msg\":\"Association Satellite Z2 lancée\"}");
  } else {
    _srv.send(500, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Échec lancement association Satellite Z2\"}");
  }
}

void Portal::handlePairSatelliteZ3() {
  if (_srv.method() != HTTP_POST) {
    _srv.send(405, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Méthode non autorisée\"}");
    return;
  }

  if (!_frisquetManager.config().useSatelliteZ3()) {
    _srv.send(400, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Satellite Z3 désactivé dans la configuration\"}");
    return;
  }

  info("[PORTAIL] Demande d'association du Satellite Z3");

  bool ok = false;
  NetworkID networkId;
  uint8_t idAssociation;

  // ⚠️ À ADAPTER selon ton API réelle :
  if (_frisquetManager.satelliteZ3().associer(networkId, idAssociation)) {
    _frisquetManager.satelliteZ3().setIdAssociation(idAssociation);
    _frisquetManager.radio().setNetworkID(networkId);
    _frisquetManager.config().setNetworkID(networkId);
    _frisquetManager.config().save();
    _frisquetManager.satelliteZ3().saveConfig();
    info("[PORTAIL] Association Satellite Z3 réussie.");
    ok = true;
  } else {
    error("[PORTAIL] Échec de l'association Satellite Z3.");
  }

  if (ok) {
    _srv.send(200, "application/json; charset=utf-8",
              "{\"ok\":true,\"msg\":\"Association Satellite Z3 lancée\"}");
  } else {
    _srv.send(500, "application/json; charset=utf-8",
              "{\"ok\":false,\"err\":\"Échec lancement association Satellite Z3\"}");
  }
}


// -------------------- Utils --------------------

void Portal::scheduleReboot(uint32_t delayMs) {
  xTaskCreatePinnedToCore([](void* d){
    uint32_t ms = (uint32_t)d;
    vTaskDelay(ms / portTICK_PERIOD_MS);
    ESP.restart();
    vTaskDelete(NULL);
  }, "rebooter", 2048, (void*)delayMs, 1, nullptr, ARDUINO_RUNNING_CORE);
}

void Portal::startAp() {
  _apRunning = true;
  WiFi.mode(WIFI_AP_STA);
  bool ok;
  if (_apPass.length() >= 8) ok = WiFi.softAP(_apSsid.c_str(), _apPass.c_str());
  else                       ok = WiFi.softAP(_apSsid.c_str()); // open
  IPAddress ip = WiFi.softAPIP();
  info("[PORTAIL] AP fallback %s (%s) %s",
       _apSsid.c_str(),
       (_apPass.length() >= 8 ? "WPA2" : "OPEN"),
       ip.toString().c_str());
}

String Portal::html() {
  return R"HTML(
<!DOCTYPE html><html lang='fr'><head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Frisquet – Configuration</title>
<style>
  :root{
    --bg:#0f1115;--card:#171a21;--muted:#8a8f98;--txt:#e7e9ee;
    --acc:#3aa3ff;--bd:#2a2f39;--ok:#1fb86a;--warn:#ffb020
  }
  *,*:before,*:after{box-sizing:border-box}
  body{margin:0;padding:24px;background:var(--bg);color:var(--txt);font:15px/1.45 system-ui,Segoe UI,Roboto,Arial}
  h1,h2{margin:0 0 12px}
  a{color:var(--acc);text-decoration:none}
  .wrap{max-width:980px;margin:0 auto;display:grid;gap:16px}
  .grid{display:grid;grid-template-columns:1fr 1fr;gap:16px}
  .grid-3{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:16px}
  .grid-2{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:16px}
  .card{background:var(--card);border:1px solid var(--bd);border-radius:12px;padding:18px;box-shadow:0 4px 16px rgba(0,0,0,.2)}
  .row{display:flex;flex-direction:column;gap:6px}
  label{font-weight:600}
  .hint{color:var(--muted);font-size:12px}
  input[type=text],input[type=password],input[type=number]{
    width:100%;padding:10px 12px;border:1px solid var(--bd);border-radius:10px;
    background:#0d1016;color:var(--txt)
  }
  .pw{position:relative}
  .pw button{
    position:absolute;right:8px;top:50%;transform:translateY(-50%);
    border:1px solid var(--bd);background:#0d1016;color:var(--muted);
    padding:4px 8px;border-radius:8px;cursor:pointer;font-size:12px
  }
  .actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:8px}
  .btn{
    display:inline-flex;align-items:center;gap:8px;border:1px solid var(--bd);
    background:#0d1016;color:var(--txt);padding:10px 14px;border-radius:10px;
    cursor:pointer;text-decoration:none
  }
  .btn.primary{background:var(--acc);color:#061019;border-color:transparent;font-weight:700}
  .badge{
    display:inline-flex;align-items:center;gap:6px;padding:6px 10px;border-radius:999px;
    border:1px solid var(--bd);background:#10131a;font-size:13px
  }
  .ok{color:var(--ok)} .warn{color:var(--warn)}
  .split{display:grid;grid-template-columns:1.3fr .7fr;gap:16px}
  .footer{color:var(--muted);font-size:12px;text-align:center;margin-top:8px}
  .msg{margin-top:10px;padding:10px;border-radius:8px;background:#111827;color:#e5e7eb;display:none}
  .msg.show{display:block}
  @media (max-width:820px){
    .grid,.grid-3, .grid-2, .split{grid-template-columns:1fr}
  }
  .btn.btn-sm{
    padding:6px 10px;
    font-size:13px;
  }
</style>
</head><body>
<div class='wrap'>

  <div class='split'>
    <div class='card'>
      <h2>Frisquet – Configuration</h2>
      <p class='hint'>
        Renseignez le Wi-Fi, le broker MQTT et les options Frisquet puis cliquez sur
        <strong>Enregistrer</strong>.
      </p>

      <form id='form' autocomplete='off'>

        <div class='card' style='background:#14171f;margin-bottom:12px'>
          <h3 style='margin:0 0 8px;font-size:15px'>Wi-Fi</h3>
          <div class='grid'>
            <div class='row'>
              <label>Nom d'hôte</label>
              <input id='wifiHostname' type='text' placeholder='esp32-device'>
              <div class='hint'>Nom utilisé sur le réseau (mDNS / logs).</div>
            </div>
            <div class='row'>
              <label>SSID Wi-Fi</label>
              <input id='wifiSsid' type='text' placeholder='MaBox'>
              <div class='hint'>Nom du réseau (2.4 GHz recommandé).</div>
            </div>
          </div>
          <div class='grid'>
            <div class='row pw'>
              <label>Mot de passe Wi-Fi</label>
              <input id='wifiPass' type='password' placeholder='••••••••'>
              <button type='button' data-toggle='#wifiPass'>Afficher</button>
            </div>
          </div>
        </div>

        <div class='card' style='background:#14171f;margin-bottom:12px'>
          <h3 style='margin:0 0 8px;font-size:15px'>MQTT</h3>
          <div class='grid-3'>
            <div class='row'>
              <label>Client ID</label>
              <input id='mqttClientId' type='text' placeholder='Heltec-Frisquet'>
            </div>
            <div class='row'>
              <label>Base topic</label>
              <input id='mqttBaseTopic' type='text' placeholder='frisquet'>
            </div>
            <div class='row'>
              <label>Hôte</label>
              <input id='mqttHost' type='text' placeholder='192.168.1.10'>
            </div>
          </div>
          <div class='grid-3' style='margin-top:8px'>
            <div class='row'>
              <label>Port</label>
              <input id='mqttPort' type='number' min='1' max='65535' placeholder='1883'>
            </div>
            <div class='row'>
              <label>Utilisateur</label>
              <input id='mqttUser' type='text' placeholder='(optionnel)'>
            </div>
            <div class='row pw'>
              <label>Mot de passe MQTT</label>
              <input id='mqttPass' type='password' placeholder='(optionnel)'>
              <button type='button' data-toggle='#mqttPass'>Afficher</button>
            </div>
          </div>
        </div>


        <div class='card' style='background:#14171f;margin-bottom:12px'>
          <h3 style='margin:0 0 8px;font-size:15px'>Frisquet</h3>
          <hr />
          <div class='row' style='margin-bottom:10px'>
            <label>NetworkID</label>
            <input id='networkID' type='text' placeholder='00:00:00:00'>
            <div class='hint'>
              Identifiant réseau au format <code>AA:BB:CC:DD</code>.
            </div>
          </div>

          <div class='grid-2'>
            <div class='row'>
              <label class='check-row'>
                <input id='useConnect' type='checkbox'>
                <span>Activer Connect</span>
                <div class='hint'>Active la passerelle Connect Frisquet.</div>
              </label>
            </div>

            <div class="row">
              <button type='button' class='btn btn-sm' id='btnPairConnect' style='margin-top:6px;display:none'>
                Associer le Connect
              </button>
            </div>
          </div>

          <div class='grid-3' style="margin-top:10px">
            <div class='row'>
              <label class='check-row'>
                <input id='useSondeExt' type='checkbox'>
                <span>Activer sonde extérieure</span>
              </label>
              <div class='hint'>Utilise la sonde extérieure radio Frisquet.</div>
            </div>
            <div class='row'>
              <label class='check-row'>
                <input id='useDS18B20' type='checkbox'>
                <span>Utiliser DS18B20</span>
              </label>
              <div class='hint'>Active l'utilisation d'un capteur de température filaire.</div>
            </div>
            <div class='row'>
              <button type='button' class='btn btn-sm' id='btnPairSondeExt' style='margin-top:6px;display:none'>
                Associer la sonde extérieure
              </button>
            </div>
          </div>

          <hr />

          <h4>Chaudières non-compatible Connect :</h4>

          <div class='grid-3' style="margin-top:10px">
            <div class='row'>
              <label class='check-row' style='margin-top:8px'>
                <input id='useSatelliteZ1' type='checkbox'>
                <span>Satellite Z1</span>
              </label>
              <div class='hint'>Active la gestion du satellite Z1.</div>

              <label class='check-row' style='margin-top:8px'>
                <input id='useSatelliteVirtualZ1' type='checkbox'>
                <span>Remplacer le satellite physique</span>
              </label>
              <div class='hint'>
                Utilise ce module à la place d’un satellite Z1 réel (émulation).
              </div>

              <button type='button' class='btn btn-sm' id='btnPairSatZ1' style='margin-top:6px;display:none'>
                Associer le Satellite Z1
              </button>
            </div>

            <div class='row'>
              <label class='check-row' style='margin-top:8px'>
                <input id='useSatelliteZ2' type='checkbox'>
                <span>Satellite Z2</span>
              </label>
              <div class='hint'>Active la gestion du satellite Z2.</div>

              <label class='check-row' style='margin-top:8px'>
                <input id='useSatelliteVirtualZ2' type='checkbox'>
                <span>Remplacer le satellite physique</span>
              </label>
              <div class='hint'>
                Utilise ce module à la place d’un satellite Z2 réel (émulation).
              </div>

              <button type='button' class='btn btn-sm' id='btnPairSatZ2' style='margin-top:6px;display:none'>
                Associer le Satellite Z2
              </button>
            </div>

            <div class='row'>
              <label class='check-row' style='margin-top:8px'>
                <input id='useSatelliteZ3' type='checkbox'>
                <span>Satellite Z3</span>
              </label>
              <div class='hint'>Active la gestion du satellite Z3.</div>

              <label class='check-row' style='margin-top:8px'>
                <input id='useSatelliteVirtualZ3' type='checkbox'>
                <span>Remplacer le satellite physique</span>
              </label>
              <div class='hint'>
                Utilise ce module à la place d’un satellite Z3 réel (émulation).
              </div>

              <button type='button' class='btn btn-sm' id='btnPairSatZ3' style='margin-top:6px;display:none'>
                Associer le Satellite Z3
              </button>
            </div>
          </div>
        </div>

        <div class='actions'>
          <button class='btn primary' type='submit'>Enregistrer</button>
          <button class='btn' type='button' id='btnReboot'>Redémarrer</button>
          <a class='btn' href='/logs'>Voir les logs</a>
          <a class='btn' href='/logs-radio'>Trames radio</a>
          <a class='btn' href='/memory'>Mémoire chaudière</a>
        </div>


        <div id='msg' class='msg'></div>
      </form>
    </div>

    <div class='card'>
      <h2>Statut</h2>
      <div class='row'>
        <span class='badge'>
          <span>Mode AP&nbsp;:</span>
          <span id='badgeAp' class='warn'>inconnu</span>
        </span>
        <span class='badge'>
          <span>Station Wi-Fi&nbsp;:</span>
          <span id='badgeSta' class='warn'>inconnu</span>
        </span>
      </div>
      <div class='row' style='margin-top:8px'>
        <div class='hint'>
          Si la station se déconnecte, un point d’accès de secours sera lancé automatiquement.
        </div>
      </div>
    </div>

  </div>

  <div class='footer'>
    Portail de configuration – Frisquet Heltec
  </div>
</div>

<script>
const $ = sel => document.querySelector(sel);
const msg = (t) => { const m=$("#msg"); if(!m) return; m.textContent=t; m.classList.add("show"); };

const FIELDS = [
  "wifiHostname","wifiSsid","wifiPass",
  "mqttHost","mqttPort","mqttUser","mqttPass",
  "mqttClientId","mqttBaseTopic",
  "networkID","useConnect","useSondeExt","useDS18B20",
  "useSatelliteZ1","useSatelliteZ2","useSatelliteZ3",
  "useSatelliteVirtualZ1","useSatelliteVirtualZ2","useSatelliteVirtualZ3"
];

function updatePairButtons() {
  const chkConnect = $("#useConnect");
  const chkSonde   = $("#useSondeExt");
  const btnConnect = $("#btnPairConnect");
  const btnSonde   = $("#btnPairSondeExt");

  if (chkConnect && btnConnect) {
    btnConnect.style.display = chkConnect.checked ? "inline-flex" : "none";
  }
  if (chkSonde && btnSonde) {
    btnSonde.style.display = chkSonde.checked ? "inline-flex" : "none";
  }


  const chkZ1 = $("#useSatelliteZ1");
  const chkZ2 = $("#useSatelliteZ2");
  const chkZ3 = $("#useSatelliteZ3");
  const btnZ1 = $("#btnPairSatZ1");
  const btnZ2 = $("#btnPairSatZ2");
  const btnZ3 = $("#btnPairSatZ3");

  if (chkZ1 && btnZ1) {
    btnZ1.style.display = chkZ1.checked ? "inline-flex" : "none";
  }
  if (chkZ2 && btnZ2) {
    btnZ2.style.display = chkZ2.checked ? "inline-flex" : "none";
  }
  if (chkZ3 && btnZ3) {
    btnZ3.style.display = chkZ3.checked ? "inline-flex" : "none";
  }
}



async function loadConfig() {
  try {
    const r = await fetch("/api/config",{cache:"no-store"});
    const j = await r.json();
    FIELDS.forEach(id => {
      const el = $("#"+id);
      if (!el || j[id] === undefined) return;

      if (el.type === "checkbox") {
        el.checked = !!j[id];       // j[id] est un booléen côté JSON
      } else {
        el.value = j[id];
      }
    });
    updatePairButtons();
  } catch(e) {
    msg("Impossible de charger la configuration.");
  }
}


async function saveConfig(e) {
  e.preventDefault();
  const fd = new FormData();
  FIELDS.forEach(id => {
    const el = $("#"+id);
    if (!el) return;

    if (el.type === "checkbox") {
      // On force "true"/"false" pour que parseBoolArg soit content
      fd.append(id, el.checked ? "true" : "false");
    } else {
      fd.append(id, el.value);
    }
  });

  try {
    const r = await fetch("/api/config", { method:"POST", body:fd });
    const j = await r.json();
    if (j.ok) {
      msg("Configuration enregistrée. Redémarrage en cours…");
      const start = Date.now();
      const tryReload = async () => {
        try {
          const rr = await fetch("/api/ping",{cache:"no-store"});
          if (rr.ok) location.reload();
          else setTimeout(tryReload, 1500);
        }
        catch(_) {
          if (Date.now()-start>25000) location.reload();
          else setTimeout(tryReload,1500);
        }
      };
      setTimeout(tryReload, 4000);
    } else {
      msg("Erreur: " + (j.err || "inconnue"));
    }
  } catch(e) {
    msg("Erreur réseau lors de l'enregistrement.");
  }
}



function setBadge(id, ok, text) {
  const el = $("#"+id);
  if (!el) return;
  el.textContent = text;
  el.className = ok ? "ok" : "warn";
}

async function loadStatus() {
  try {
    const r = await fetch("/api/status", { cache: "no-store" });
    const j = await r.json();

    // Mode AP
    setBadge("badgeAp", j.apRunning, j.apRunning ? "actif" : "inactif");

    // Station Wi-Fi
    let staText;
    if (j.staConnected) {
      if (j.ip && j.ip.length) {
        staText = "connectée (" + j.ip + ")";
      } else {
        staText = "connectée";
      }
    } else {
      staText = "déconnectée";
    }
    setBadge("badgeSta", j.staConnected, staText);
  } catch (e) {
    setBadge("badgeAp", false, "indisponible");
    setBadge("badgeSta", false, "indisponible");
  }
}

async function pairConnect() {
  try {
    msg("Lancement de l'association du Connect…");
    const r = await fetch("/api/connect/pair", { method:"POST" });
    const j = await r.json();
    if (j.ok) {
      msg(j.msg || "Association Connect lancée. Consultez les logs RADIO.");
    } else {
      msg("Erreur association Connect : " + (j.err || "inconnue"));
    }
  } catch (e) {
    msg("Erreur réseau lors de l'association Connect.");
  }
}

async function pairSondeExt() {
  try {
    msg("Lancement de l'association de la sonde extérieure…");
    const r = await fetch("/api/sonde-ext/pair", { method:"POST" });
    const j = await r.json();
    if (j.ok) {
      msg(j.msg || "Association sonde extérieure lancée. Consultez les logs RADIO.");
    } else {
      msg("Erreur association sonde extérieure : " + (j.err || "inconnue"));
    }
  } catch (e) {
    msg("Erreur réseau lors de l'association de la sonde extérieure.");
  }
}

async function pairSatellite(zone) {
  try {
    msg("Lancement de l'association du Satellite " + zone + "…");
    const r = await fetch("/api/satellite/z" + zone.toLowerCase() + "/pair", { method:"POST" });
    const j = await r.json();
    if (j.ok) {
      msg(j.msg || ("Association Satellite " + zone + " lancée. Consultez les logs RADIO."));
    } else {
      msg("Erreur association Satellite " + zone + " : " + (j.err || "inconnue"));
    }
  } catch (e) {
    msg("Erreur réseau lors de l'association Satellite " + zone + ".", false);
  }
}

document.addEventListener("DOMContentLoaded", ()=>{
  // Toggle password
  document.querySelectorAll('[data-toggle]').forEach(btn=>{
    btn.addEventListener('click',()=>{
      const sel=btn.getAttribute('data-toggle');
      const inp=document.querySelector(sel);
      if(!inp) return;
      inp.type = (inp.type==='password') ? 'text' : 'password';
      btn.textContent = (inp.type==='password') ? 'Afficher' : 'Masquer';
    });
  });

  const form = $("#form");
  if (form) form.addEventListener("submit", saveConfig);

  const btnReboot = $("#btnReboot");
  if (btnReboot) {
    btnReboot.addEventListener("click", async ()=>{
      msg("Redémarrage…");
      try { await fetch("/api/reboot", {method:"POST"}); } catch(_) {}
      setTimeout(()=>location.reload(), 7000);
    });
  }

  const chkConnect = $("#useConnect");
  const chkSonde   = $("#useSondeExt");
  const chkZ1      = $("#useSatelliteZ1");
  const chkZ2      = $("#useSatelliteZ2");
  const chkZ3      = $("#useSatelliteZ3");

  if (chkConnect) chkConnect.addEventListener("change", updatePairButtons);
  if (chkSonde)   chkSonde.addEventListener("change", updatePairButtons);
  if (chkZ1)      chkZ1.addEventListener("change", updatePairButtons);
  if (chkZ2)      chkZ2.addEventListener("change", updatePairButtons);
  if (chkZ3)      chkZ3.addEventListener("change", updatePairButtons);

  const btnPairConnect = $("#btnPairConnect");
  const btnPairSonde   = $("#btnPairSondeExt");
  const btnPairSatZ1   = $("#btnPairSatZ1");
  const btnPairSatZ2   = $("#btnPairSatZ2");
  const btnPairSatZ3   = $("#btnPairSatZ3");

  if (btnPairConnect) btnPairConnect.addEventListener("click", pairConnect);
  if (btnPairSonde)   btnPairSonde.addEventListener("click", pairSondeExt);
  if (btnPairSatZ1)   btnPairSatZ1.addEventListener("click", ()=>pairSatellite("1"));
  if (btnPairSatZ2)   btnPairSatZ2.addEventListener("click", ()=>pairSatellite("2"));
  if (btnPairSatZ3)   btnPairSatZ3.addEventListener("click", ()=>pairSatellite("3"));

  loadConfig();
  loadStatus();
  setInterval(loadStatus, 5000);
});
</script>

</body></html>
)HTML";
}



String Portal::logsHtml() {
  return R"HTML(
<!DOCTYPE html><html lang='fr'><head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Frisquet – Logs</title>
<style>
  :root{
    --bg:#0f1115;--card:#171a21;--muted:#8a8f98;--txt:#e7e9ee;
    --acc:#3aa3ff;--bd:#2a2f39;--ok:#1fb86a;--warn:#ffb020
  }
  *,*:before,*:after{box-sizing:border-box}
  body{
    margin:0;padding:24px;background:var(--bg);color:var(--txt);
    font:15px/1.45 system-ui,Segoe UI,Roboto,Arial
  }
  a{color:var(--acc);text-decoration:none}
  h1,h2{margin:0 0 12px}
  .wrap{max-width:980px;margin:0 auto;display:grid;gap:16px}
  .card{
    background:var(--card);border:1px solid var(--bd);border-radius:12px;
    padding:18px;box-shadow:0 4px 16px rgba(0,0,0,.2)
  }
  .toolbar{
    display:flex;flex-wrap:wrap;gap:10px;align-items:center;
    margin:8px 0 12px
  }
  .badge{
    display:inline-flex;align-items:center;gap:6px;
    padding:6px 10px;border-radius:999px;border:1px solid var(--bd);
    background:#10131a;font-size:13px
  }
  .btn{
    display:inline-flex;align-items:center;gap:8px;
    border:1px solid var(--bd);background:#0d1016;color:var(--txt);
    padding:10px 14px;border-radius:10px;cursor:pointer;
    text-decoration:none
  }
  .btn.primary{
    background:var(--acc);color:#061019;border-color:transparent;
    font-weight:700
  }
  label{
    color:var(--muted);font-weight:600;font-size:13px;
    display:flex;align-items:center;gap:6px
  }
  select,input[type=text],input[type=checkbox]{
    padding:8px 10px;border:1px solid var(--bd);border-radius:10px;
    background:#0d1016;color:var(--txt)
  }
  pre{
    white-space:pre-wrap;background:#0b0e13;color:#e6e6e6;
    padding:12px;border-radius:10px;max-height:70vh;overflow:auto;
    margin:0;font-family:ui-monospace,Menlo,Consolas,monospace
  }
  .muted{color:var(--muted)}
  .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
  .topnav{display:flex;align-items:center;gap:10px;margin-bottom:4px}
  @media (max-width:820px){.toolbar{gap:8px}}
  .check-row{
    display:flex;
    align-items:center;
    gap:8px;
    font-weight:600;
  }
  input[type=checkbox]{
    width:auto;
    accent-color:var(--acc);
  }
</style>
</head><body>
<div class='wrap'>

  <div class='topnav'>
    <a href='/' class='btn'>&larr;&nbsp;Retour</a>
    <span class='badge'>Frisquet – Logs</span>
  </div>

  <div class='card'>
    <div class='toolbar'>
      <label>Rafraîchissement
        <select id='refresh'>
          <option value='0'>Off</option>
          <option value='1000'>1s</option>
          <option value='2000' selected>2s</option>
          <option value='5000'>5s</option>
          <option value='10000'>10s</option>
        </select>
      </label>

      <label>Niveau
        <select id='level'>
          <option value=''>Tous</option>
          <option value='INFO'>INFO</option>
          <option value='DEBUG'>DEBUG</option>
          <option value='WARN'>WARN</option>
          <option value='ERROR'>ERROR</option>
        </select>
      </label>

      <label>Filtre
        <input id='filter' type='text' placeholder='rechercher...'>
      </label>

      <label>Lignes
        <select id='limit'>
          <option value='0' selected>Toutes</option>
          <option value='200'>200</option>
          <option value='500'>500</option>
        </select>
      </label>

      <label>
        <input id='autoscroll' type='checkbox' checked>
        Auto-scroll
      </label>

      <button id='btnReload' class='btn'>Recharger</button>
      <button id='btnClear' class='btn'>Effacer</button>
    </div>

    <pre id='log'>(chargement...)</pre>
  </div>

  <div class='muted' style='text-align:center'>
    Astuce : filtre par niveau (p. ex. <code>ERROR</code>) et limite pour ne voir que la fin du journal.
  </div>

</div>

<script>
const $ = s => document.querySelector(s);
let raw = "";
let timer = null;

const elLog     = $("#log");
const selRef    = $("#refresh");
const selLvl    = $("#level");
const selLimit  = $("#limit");
const inpFilter = $("#filter");
const cbAuto    = $("#autoscroll");
const btnReload = $("#btnReload");
const btnClear  = $("#btnClear");

function applyFilters(txt){
  if(!txt) return "";
  let lines = txt.split("\n");

  const lvl = selLvl.value.trim();
  const f   = inpFilter.value.trim().toLowerCase();

  if(lvl){
    lines = lines.filter(l => l.includes(lvl));
  }
  if(f){
    lines = lines.filter(l => l.toLowerCase().includes(f));
  }

  const lim = parseInt(selLimit.value||"0",10);
  if(lim>0 && lines.length>lim){
    lines = lines.slice(-lim);
  }

  return lines.join("\n");
}

function render(){
  const out = applyFilters(raw);
  elLog.textContent = out || "(vide)";
  if(cbAuto.checked){
    elLog.scrollTop = elLog.scrollHeight;
  }
}

async function reload(){
  try{
    // On envoie la limite et éventuellement le niveau au backend
    const limitParam = selLimit.value === "0" ? "500" : selLimit.value;
    const lvl = selLvl.value.trim();
    const qs =
      "?limit=" + encodeURIComponent(limitParam) +
      (lvl ? "&level="+encodeURIComponent(lvl) : "") +
      "&_=" + Date.now();

    const r = await fetch("/api/logs"+qs,{cache:"no-store"});
    const arr = await r.json();   // ["ligne1", "ligne2", ...]
    raw = arr.join("\n");
    render();
  }catch(e){
    elLog.textContent = "Erreur chargement logs: " + e;
  }
}

function updateRefreshTimer(){
  if(timer){ clearInterval(timer); timer = null; }
  const v = parseInt(selRef.value||"0",10);
  if(v>0){
    timer = setInterval(reload, v);
  }
}

document.addEventListener("DOMContentLoaded", ()=>{
  btnReload.addEventListener("click", reload);
  btnClear.addEventListener("click", async ()=>{
    try{
      await fetch("/api/logs/clear",{method:"POST"});
      raw = "";
      render();
    }catch(e){
      console.error("Erreur clear logs", e);
    }
  });

  [selLvl, inpFilter, selLimit].forEach(el=>{
    el.addEventListener("input", render);
  });

  selRef.addEventListener("change", updateRefreshTimer);

  reload();
  updateRefreshTimer();
});
</script>

</body></html>
)HTML";
}


String Portal::memoryHtml() {
  return R"HTML(
<!DOCTYPE html><html lang='fr'><head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Frisquet – Mémoire chaudière</title>
<style>
  :root{
    --bg:#0f1115;--card:#171a21;--muted:#8a8f98;--txt:#e7e9ee;
    --acc:#3aa3ff;--bd:#2a2f39;--ok:#1fb86a;--warn:#ffb020
  }
  *,*:before,*:after{box-sizing:border-box}
  body{
    margin:0;padding:24px;background:var(--bg);color:var(--txt);
    font:15px/1.45 system-ui,Segoe UI,Roboto,Arial
  }
  a{color:var(--acc);text-decoration:none}
  h1,h2{margin:0 0 12px}
  .wrap{max-width:980px;margin:0 auto;display:grid;gap:16px}
  .card{
    background:var(--card);border:1px solid var(--bd);border-radius:12px;
    padding:18px;box-shadow:0 4px 16px rgba(0,0,0,.2)
  }
  .toolbar{
    display:flex;flex-wrap:wrap;gap:10px;align-items:center;
    margin:8px 0 12px
  }
  .badge{
    display:inline-flex;align-items:center;gap:6px;
    padding:6px 10px;border-radius:999px;border:1px solid var(--bd);
    background:#10131a;font-size:13px
  }
  .btn{
    display:inline-flex;align-items:center;gap:8px;
    border:1px solid var(--bd);background:#0d1016;color:var(--txt);
    padding:10px 14px;border-radius:10px;cursor:pointer;
    text-decoration:none
  }
  .btn.primary{
    background:var(--acc);color:#061019;border-color:transparent;
    font-weight:700
  }
  label{
    color:var(--muted);font-weight:600;font-size:13px;
    display:flex;align-items:center;gap:6px
  }
  input[type=text],input[type=number],select{
    padding:8px 10px;border:1px solid var(--bd);border-radius:10px;
    background:#0d1016;color:var(--txt)
  }
  pre{
    white-space:pre;background:#0b0e13;color:#e6e6e6;
    padding:12px;border-radius:10px;max-height:70vh;overflow:auto;
    margin:0;font-family:ui-monospace,Menlo,Consolas,monospace
  }
  .muted{color:var(--muted)}
  .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
  .topnav{display:flex;align-items:center;gap:10px;margin-bottom:4px}
  .msg{
    margin-top:10px;padding:10px 12px;border-radius:10px;
    background:#0b0e13;border:1px solid var(--bd);display:none
  }
  .msg.show{display:block}
  .msg.ok{border-color:var(--ok);color:var(--ok)}
  .msg.err{border-color:#ff5a5a;color:#ff8a8a}
</style>
</head><body>
<div class='wrap'>

  <div class='topnav'>
    <a href='/' class='btn'>&larr;&nbsp;Retour</a>
    <span class='badge'>Frisquet – Mémoire chaudière</span>
  </div>

  <div class='card'>
    <div class='toolbar'>
      <label>Adresse (hex)
        <input id='start' type='text' value='0000' size='6'>
      </label>
      <label>Longueur (mots 16-bit)
        <input id='len' type='number' min='1' max='256' value='64'>
      </label>
      <label>
        <input id='auto' type='checkbox'>
        Auto +len
      </label>
      <label>Intervalle
        <select id='refresh'>
          <option value='0'>Off</option>
          <option value='500'>500 ms</option>
          <option value='1000'>1s</option>
          <option value='2000'>2s</option>
          <option value='5000'>5s</option>
        </select>
      </label>
      <button class='btn primary' id='btnRead'>Lire</button>
      <button class='btn' id='btnStop'>Stop</button>
    </div>
    <div class='row muted'>
      Utilise l'association Connect pour interroger la mémoire chaudière.
    </div>
    <pre id='dump'>(aucune donnée)</pre>
    <div id='msg' class='msg'></div>
  </div>

</div>

<script>
const $ = s => document.querySelector(s);
const dump = $("#dump");
const msg = $("#msg");
const inpStart = $("#start");
const inpLen = $("#len");
const cbAuto = $("#auto");
const selRef = $("#refresh");
const btnRead = $("#btnRead");
const btnStop = $("#btnStop");
let timer = null;

function toHex(n, w){
  return n.toString(16).toUpperCase().padStart(w, "0");
}

function showMsg(text, ok){
  if(!msg) return;
  msg.textContent = text;
  msg.className = "msg show " + (ok ? "ok" : "err");
}

function renderDump(startHex, words){
  if(!words || !words.length){
    dump.textContent = "(aucune donnée)";
    return;
  }
  const start = parseInt(startHex, 16) || 0;
  const lines = [];
  for(let i=0; i<words.length; i+=8){
    const addr = start + i;
    const chunk = words.slice(i, i+8).map(w => w === "??" ? "??" : w);
    lines.push(toHex(addr,4) + ": " + chunk.join(" "));
  }
  dump.textContent = lines.join("\n");
}

async function readOnce(){
  const start = (inpStart.value || "0000").trim();
  const len = parseInt(inpLen.value || "16", 10);
  const qs = "?start=" + encodeURIComponent(start) +
             "&len=" + encodeURIComponent(len) +
             "&_=" + Date.now();
  try{
    const r = await fetch("/api/memory" + qs, { cache:"no-store" });
    const j = await r.json();
    if(!j.ok){
      showMsg(j.err || "Erreur lecture", false);
      return;
    }
    renderDump(j.startHex || start, j.words || []);
    if(j.errors && j.errors.length){
      showMsg("Lecture partielle: " + j.errors.length + " erreurs.", false);
    } else {
      showMsg("Lecture OK (" + (j.words ? j.words.length : 0) + " mots).", true);
    }
    if(cbAuto.checked && j.startHex && j.words){
      const next = (parseInt(j.startHex,16) + j.words.length) & 0xFFFF;
      inpStart.value = toHex(next, 4);
    }
  }catch(e){
    showMsg("Erreur réseau: " + e, false);
  }
}

function updateTimer(){
  if(timer){ clearInterval(timer); timer = null; }
  const v = parseInt(selRef.value || "0", 10);
  if(v > 0){
    timer = setInterval(readOnce, v);
  }
}

btnRead.addEventListener("click", readOnce);
btnStop.addEventListener("click", ()=>{
  selRef.value = "0";
  updateTimer();
});
selRef.addEventListener("change", updateTimer);
</script>

</body></html>
)HTML";
}


String Portal::logsRadioHtml() {
  return R"HTML(
<!DOCTYPE html><html lang='fr'><head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Frisquet – Trames radio</title>
<style>
  :root{
    --bg:#0f1115;--card:#171a21;--muted:#8a8f98;--txt:#e7e9ee;
    --acc:#3aa3ff;--bd:#2a2f39;--ok:#1fb86a;--warn:#ffb020
  }
  *,*:before,*:after{box-sizing:border-box}
  body{
    margin:0;padding:24px;background:var(--bg);color:var(--txt);
    font:15px/1.45 system-ui,Segoe UI,Roboto,Arial
  }
  a{color:var(--acc);text-decoration:none}
  h1,h2{margin:0 0 12px}
  .wrap{max-width:1280px;margin:0 auto;display:grid;gap:16px}
  .card{
    background:var(--card);border:1px solid var(--bd);border-radius:12px;
    padding:18px;box-shadow:0 4px 16px rgba(0,0,0,.2)
  }
  .toolbar{
    display:flex;flex-wrap:wrap;gap:10px;align-items:center;
    margin:8px 0 12px
  }
  .badge{
    display:inline-flex;align-items:center;gap:6px;
    padding:6px 10px;border-radius:999px;border:1px solid var(--bd);
    background:#10131a;font-size:13px
  }
  .btn{
    display:inline-flex;align-items:center;gap:8px;
    border:1px solid var(--bd);background:#0d1016;color:var(--txt);
    padding:10px 14px;border-radius:10px;cursor:pointer;
    text-decoration:none
  }
  .btn.primary{
    background:var(--acc);color:#061019;border-color:transparent;
    font-weight:700
  }
  label{
    color:var(--muted);font-weight:600;font-size:13px;
    display:flex;align-items:center;gap:6px
  }
  select,input[type=text],input[type=checkbox]{
    padding:8px 10px;border:1px solid var(--bd);border-radius:10px;
    background:#0d1016;color:var(--txt)
  }
  #payload {
    width:100%;
  }
  pre{
    font-size:80%;
    white-space:pre-wrap;background:#0b0e13;color:#e6e6e6;
    padding:12px;border-radius:10px;max-height:60vh;overflow:auto;
    margin:0;font-family:ui-monospace,Menlo,Consolas,monospace
  }
  .muted{color:var(--muted)}
  .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
  .topnav{display:flex;align-items:center;gap:10px;margin-bottom:4px}
  .msg{
    margin-top:8px;padding:8px 10px;border-radius:8px;
    background:#111827;color:#e5e7eb;display:none;font-size:13px
  }
  .msg.show{display:block}
  .msg.err{border:1px solid #b91c1c}
  .msg.ok{border:1px solid #15803d}
  @media (max-width:820px){.toolbar{gap:8px}}
</style>
</head><body>
<div class='wrap'>

  <div class='topnav'>
    <a href='/' class='btn'>&larr;&nbsp;Retour</a>
    <span class='badge'>Frisquet – Trames radio</span>
    <a href='/logs' class='btn'>Tous les logs</a>
  </div>

  <div class='card'>
    <h2>Trames radio (niveau RADIO)</h2>
    <div class='toolbar'>
      <label>Rafraîchissement
        <select id='refresh'>
          <option value='0'>Off</option>
          <option value='1000'>1s</option>
          <option value='2000' selected>2s</option>
          <option value='5000'>5s</option>
          <option value='10000'>10s</option>
        </select>
      </label>

      <label>Filtre texte
        <input id='filter' type='text' placeholder='rechercher...'>
      </label>

      <label>Lignes
        <select id='limit'>
          <option value='0' selected>Toutes</option>
          <option value='200'>200</option>
          <option value='500'>500</option>
        </select>
      </label>

      <label>
        <input id='autoscroll' type='checkbox' checked>
        Auto-scroll
      </label>

      <button id='btnReload' class='btn'>Recharger</button>
      <button id='btnClear' class='btn'>Effacer</button>
    </div>

    <pre id='log'>(chargement...)</pre>
  </div>

  <div class='card'>
    <h2>Envoyer une trame radio</h2>
    <div class='row'>
      <label for='payload'>Payload hexadécimal</label>
      <input id='payload' type='text' placeholder='ex: A5 01 02 0F 3C'>
    </div>
    <div class='hint muted' style='margin-top:4px'>
      Format : uniquement 0-9, A-F, a-f et espaces. Les espaces sont ignorés.
    </div>
    <div class='row' style='margin-top:10px'>
      <button id='btnSend' class='btn primary'>Envoyer</button>
    </div>
    <div id='msg' class='msg'></div>
  </div>

  <div class='muted' style='text-align:center'>
    Cette page affiche uniquement les logs avec le niveau <code>RADIO</code>
    (filtrés côté backend via <code>level=RADIO</code>).
  </div>

</div>

<script>
const $ = s => document.querySelector(s);
let raw = "";
let timer = null;

const elLog     = $("#log");
const selRef    = $("#refresh");
const selLimit  = $("#limit");
const inpFilter = $("#filter");
const cbAuto    = $("#autoscroll");
const btnReload = $("#btnReload");
const btnClear  = $("#btnClear");

const inpPayload = $("#payload");
const btnSend    = $("#btnSend");
const msgBox     = $("#msg");

function showMsg(text, ok){
  if(!msgBox) return;
  msgBox.textContent = text;
  msgBox.className = "msg show " + (ok ? "ok" : "err");
}

function applyFilters(txt){
  if(!txt) return "";
  let lines = txt.split("\n");

  const f   = inpFilter.value.trim().toLowerCase();
  if(f){
    lines = lines.filter(l => l.toLowerCase().includes(f));
  }

  const lim = parseInt(selLimit.value||"0",10);
  if(lim>0 && lines.length>lim){
    lines = lines.slice(-lim);
  }

  return lines.join("\n");
}

function render(){
  const out = applyFilters(raw);
  elLog.textContent = out || "(vide)";
  if(cbAuto.checked){
    elLog.scrollTop = elLog.scrollHeight;
  }
}

async function reload(){
  try{
    const limitParam = selLimit.value === "0" ? "500" : selLimit.value;
    const qs =
      "?limit=" + encodeURIComponent(limitParam) +
      "&level=RADIO&_=" + Date.now();

    const r = await fetch("/api/logs"+qs,{cache:"no-store"});
    const arr = await r.json();   // ["ligne1", "ligne2", ...]
    raw = arr.join("\n");
    render();
  }catch(e){
    elLog.textContent = "Erreur chargement logs: " + e;
  }
}

function updateRefreshTimer(){
  if(timer){ clearInterval(timer); timer = null; }
  const v = parseInt(selRef.value||"0",10);
  if(v>0){
    timer = setInterval(reload, v);
  }
}

async function sendPayload(){
  const hex = (inpPayload.value || "").trim();
  if(!hex){
    showMsg("Payload vide.", false);
    return;
  }

  // petite validation côté front
  if(!/^[0-9a-fA-F ]+$/.test(hex)){
    showMsg("Payload invalide : utilisez uniquement 0-9, A-F et espaces.", false);
    return;
  }

  try{
    const fd = new FormData();
    fd.append("payload", hex);

    const r = await fetch("/api/radio/send", {
      method:"POST",
      body: fd
    });

    const txt = await r.text();
    try {
      const j = JSON.parse(txt);
      if(j.ok){
        showMsg("Trame envoyée (voir logs RADIO).", true);
        // éventuellement on recharge les logs
        reload();
      } else {
        showMsg("Erreur envoi trame : " + (j.err || "inconnue"), false);
      }
    } catch(_){
      showMsg("Réponse inattendue du serveur: " + txt, false);
    }
  }catch(e){
    showMsg("Erreur réseau lors de l'envoi: " + e, false);
  }
}

document.addEventListener("DOMContentLoaded", ()=>{
  btnReload.addEventListener("click", reload);
  btnClear.addEventListener("click", async ()=>{
    try{
      await fetch("/api/logs/clear",{method:"POST"});
      raw = "";
      render();
      showMsg("Logs effacés.", true);
    }catch(e){
      console.error("Erreur clear logs", e);
      showMsg("Erreur lors de l'effacement des logs.", false);
    }
  });

  [inpFilter, selLimit].forEach(el=>{
    el.addEventListener("input", render);
  });

  selRef.addEventListener("change", updateRefreshTimer);

  if(btnSend){
    btnSend.addEventListener("click", sendPayload);
  }

  reload();
  updateRefreshTimer();
});
</script>

</body></html>
)HTML";
}

bool Portal::hexStringToBufferRaw(const String& hex, uint8_t* buffer, size_t maxLen, size_t& outLen) {
    outLen = 0;

    String s = hex;
    s.trim();
    s.replace(" ", "");
    s.replace("\n", "");
    s.replace("\t", "");

    if (s.length() == 0) return true;      // OK, buffer vide
    if (s.length() % 2 != 0) return false; // longueur impaire

    size_t needed = s.length() / 2;
    if (needed > maxLen) return false;     // dépasse la taille fournie

    for (size_t i = 0; i < s.length(); i += 2) {
        char c1 = s[i];
        char c2 = s[i+1];

        if (!isxdigit(c1) || !isxdigit(c2)) {
            return false;
        }

        buffer[outLen++] = strtol(s.substring(i, i+2).c_str(), nullptr, 16);
    }

    return true;
}
