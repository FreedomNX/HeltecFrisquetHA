#pragma once

#include "Frisquet/FrisquetRadio.h"
#include "Config.h"
#include "MQTT/MqttManager.h"
#include "Frisquet/SondeExterieure.h"
#include "Frisquet/Connect.h"
#include "Logs.h"
#include "DS18B20.h"
#include "Frisquet/Satellite.h"
#include "Frisquet/Zone.h"

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
  Satellite& satelliteZ1() { return _satelliteZ1; }
  Satellite& satelliteZ2() { return _satelliteZ2; }
  Satellite& satelliteZ3() { return _satelliteZ3; }

private:
  FrisquetRadio& _radio;
  Config&        _cfg;
  MqttManager&   _mqtt;

  Zone _zone1;
  Zone _zone2;
  Zone _zone3;

  SondeExterieure _sondeExterieure;
  Connect _connect;

  Satellite _satelliteZ1;
  Satellite _satelliteZ2;
  Satellite _satelliteZ3;

  DS18B20* _ds18b20;

  void onRadioReceive();


  bool _envoiZ1 = false;
  bool _envoiZ2 = false;
  bool _envoiZ3 = false;

  // MQTT
  MqttDevice _device;

  
};
