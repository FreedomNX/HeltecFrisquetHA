#pragma once

#include "Frisquet/FrisquetRadio.h"
#include "Config.h"
#include "MQTT/MqttManager.h"
#include "Frisquet/SondeExterieure.h"
#include "Frisquet/Connect.h"
#include "Logs.h"
#include "DS18B20.h"

class FrisquetManager {
public:
  FrisquetManager(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt);

  void begin();
  void loop();

  void initMqtt();
  void initDS18B20();

  FrisquetRadio& radio() { return _radio; }
  MqttManager& mqtt() { return _mqtt; }
  Config& config() { return _cfg; }
  Connect& connect() { return _connect; }
  SondeExterieure& sondeExterieure() { return _sondeExterieure; }

private:
  FrisquetRadio& _radio;
  Config&        _cfg;
  MqttManager&   _mqtt;

  SondeExterieure _sondeExterieure;
  Connect _connect;
  DS18B20* _ds18b20;

  void onRadioReceive();


  bool _envoiZ1 = false;
  bool _envoiZ2 = false;
  bool _envoiZ3 = false;

  // MQTT
  MqttDevice _device;

  
};
