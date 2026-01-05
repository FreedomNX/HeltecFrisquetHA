#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Config.h"
#include "Logs.h"
#include "FrisquetManager.h"

// Portail Web de configuration + logs (adapté à ton Config)
class Portal {
public:
  explicit Portal(FrisquetManager& frisquetManager, uint16_t port = 80);

  // startApFallbackIfNoWifi = monte un AP "ESP32-Setup" si pas de Wi-Fi actif
  void begin(bool startApFallbackIfNoWifi = false);
  void loop();

private:
  WebServer _srv;
  FrisquetManager& _frisquetManager;

  // AP fallback
  bool _apRunning = false;
  String _apSsid = "HeltecFrisquet-Setup";
  String _apPass = "frisquetconfig";

  // Handlers
  void handleIndex();            // GET /
  void handlePing();             // GET /api/ping
  void handleGetConfig();        // GET /api/config
  void handlePostConfig();       // POST /api/config
  void handleReboot();           // POST /api/reboot
  void handleGetLogs();          // GET /api/logs
  void handleLogsPage();         // GET /logs
  void handleClearLogs();        // GET /logs/clear
  void handleMemoryRead();       // GET /api/memory
  void handleMemoryScan();       // GET /api/memory/scan
  void handleMemoryPage();       // GET /memory
  void handleStatus();
  void handleRadioLogsPage();
  void handleSendRadio();
  void handlePairConnect();
  void handlePairSondeExt();
  void handlePairSatelliteZ1();
  void handlePairSatelliteZ2();
  void handlePairSatelliteZ3();


  // Utils
  static String html();
  static String logsHtml();
  static String memoryHtml();
  String logsRadioHtml();
  void scheduleReboot(uint32_t delayMs = 800);
  bool hexStringToBufferRaw(const String& hex, uint8_t* buffer, size_t maxLen, size_t& outLen);

  // AP
  void startAp();
};
