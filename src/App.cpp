#include "App.h"

App::App() : _mqtt(_wifiClient), _frisquetManager(_radio, _cfg, _mqtt) {}

void App::begin() {
  Serial.begin(115200);
  delay(50);

  Heltec.begin(true /*DisplayEnable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  initDisplay();

  initConfig();
  initNetwork();
  initMqtt();
  initPortal();
  initOta();

  _frisquetManager.begin();
}

void App::loop() {
  // boucle des services
  _networkManager.loop();
  _ota.loop();
  _portal->loop();
  _mqtt.loop();
  _frisquetManager.loop();
  updateDisplay();
  delay(10);
}

void App::initConfig() {
  _cfg.load();
}

void App::initNetwork() {
    _networkManager.onConnected([&](){
        info("[WIFI] CONNECTED  IP=%s  RSSI=%ddBm\n", _networkManager.ipStr().c_str(), _networkManager.rssi());
        updateDisplay(true);
        //WiFi.mode(WIFI_STA);
    });
    _networkManager.onDisconnected([&](const String& reason){
        info("[WIFI] DISCONNECTED (%s)\n", reason.c_str());
        updateDisplay(true);
        //WiFi.mode(WIFI_AP_STA);
    });

    _networkManager.begin(_cfg.getWiFiOptions());
}

void App::initMqtt() {
  _mqtt.begin(_cfg.getMQTTOptions());
}

void App::initPortal() {
  _portal = new Portal(_frisquetManager);
  _portal->begin(/*startApFallbackIfNoWifi=*/true);

  info("[PORTAIL] Portail initialisé.");
}

void App::initOta() {
  _ota.begin(_networkManager.hostname().c_str());
}

void App::initDisplay() {
  if (!Heltec.display) {
    return;
  }

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, "Heltec Frisquet");
  Heltec.display->drawString(0, 12, "Boot...");
  Heltec.display->display();
  _lastDisplayMs = millis();
}

void App::updateDisplay(bool force) {
  if (!Heltec.display) {
    return;
  }

  uint32_t now = millis();
  if (!force && (now - _lastDisplayMs) < 1000) {
    return;
  }
  _lastDisplayMs = now;

  const bool wifiOk = _networkManager.isConnected();
  const bool mqttOk = _mqtt.connected();

  String wifiLine = String("WiFi: ") + (wifiOk ? "OK" : "OFF");
  String mqttLine = String("MQTT: ") + (mqttOk ? "OK" : "OFF");
  String ipLine = String("IP: ") + (wifiOk ? _networkManager.ipStr() : "-");
  String rssiLine = String("RSSI: ") + (wifiOk ? String(_networkManager.rssi()) + " dBm" : "-");
  String ssidLine = String("SSID: ") + (wifiOk ? _networkManager.ssid() : "-");

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, wifiLine);
  Heltec.display->drawString(0, 12, mqttLine);
  Heltec.display->drawString(0, 24, ipLine);
  Heltec.display->drawString(0, 36, rssiLine);
  Heltec.display->drawStringMaxWidth(0, 48, 128, ssidLine);
  Heltec.display->display();
}
