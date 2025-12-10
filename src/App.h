#pragma once

#include <Arduino.h>
#include <WiFiClient.h>
#include "MQTT/MqttManager.h"
#include "MQTT/MqttDevice.h"
#include "Config.h"
#include "NetworkManager.h"
#include "Portal.h"
#include "Logs.h"
#include "OTA.h"

#include "Frisquet/FrisquetRadio.h"
#include "FrisquetManager.h"

class App {
public:
  App();

  void begin();
  void loop();

private:
  // Dépendances et services
  Config _cfg;
  WiFiClient _wifiClient;
  MqttManager _mqtt;
  NetworkManager _networkManager;
  OTA _ota;

  FrisquetManager _frisquetManager;

  // Portail
  Portal* _portal = nullptr;

  // Radio
  FrisquetRadio _radio;

  // Étapes
  void initConfig();
  void initNetwork();
  void initMqtt();
  void initPortal();
  void initOta();
};
