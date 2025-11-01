#include "mqtt.h"
#include "conf.h"

#include "Portal.h"

Config cfg;

MQTT::MQTT() : client(this->wifiClient) {
    
}

void MQTT::init() {
    // Initialisation de la connexion MQTT
  this->client.setServer(cfg.mqttHost, cfg.mqttPort);
  this->client.setBufferSize(2048);
  
  if(this->connectToMQTT()) {
    this->connectTopics();
  }
}

boolean MQTT::connectToMQTT() {
  if (!this->client.connected()) {
    client.setKeepAlive(30);
    DBG_PRINTLN(F("Connexion au service MQTT..."));
    if (this->client.connect("ESP32 Frisquet", cfg.mqttUser, cfg.mqttPass)) {
      DBG_PRINTLN(F("Connection MQTT établie."));
      return true;
    } 
  } else {
    return true;
  }

  return false;
}

boolean MQTT::connected() {
  return this->client.connected();
}

void MQTT::connecterTemperatureAmbianteZ1() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempAmbiante1";
  char name[] = "Temperature ambiante Z1";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneZ1() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempConsigne1";
  char name[] = "Temperature consigne Z1";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureAmbianteZ2() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempAmbiante2";
  char name[] = "Temperature ambiante Z2";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneZ2() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempConsigne2";
  char name[] = "Temperature consigne Z2";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureAmbianteZ3() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempAmbiante3";
  char name[] = "Temperature ambiante Z3";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneZ3() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempConsigne3";
  char name[] = "Temperature consigne Z3";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureExterieure() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempExterieure";
  char name[] = "Temperature exterieure";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureCDC() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempCDC";
  char name[] = "Temperature corps de chauffe";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureDepartZ1() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempDepart1";
  char name[] = "Temperature depart Z1";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureECS() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "tempECS";
  char name[] = "Temperature ECS";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterAssociationSondeExterieure() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "asssonde";
  char name[] = "Associer Sonde Ext";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/switch/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/switch/frisquet/%s/state",
  "command_topic": "homeassistant/switch/frisquet/%s/set",
  "payload_on": "ON",
  "payload_off": "OFF",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
  //this->client.publish(ASS_SON_TOPIC, "OFF");
}

void MQTT::connecterAssociationConnect() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "assconnect";
  char name[] = "Associer Connect";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/switch/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/switch/frisquet/%s/state",
  "command_topic": "homeassistant/switch/frisquet/%s/set",
  "payload_on": "ON",
  "payload_off": "OFF",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
  //this->client.publish(ASS_CON_TOPIC, "OFF");
}

void MQTT::connecterEffacerNVS() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "erasenvs";
  char name[] = "Effacer NVS";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/switch/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/switch/frisquet/%s/state",
  "command_topic": "homeassistant/switch/frisquet/%s/set",
  "payload_on": "ON",
  "payload_off": "OFF",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
  //this->client.publish(RES_NVS_TOPIC, "OFF");
}

void MQTT::connecterPayload() {
  char configTopic[60];
  char configPayload[350];
  char id[] = "payload";
  char name[] = "Payload";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterModeZ1() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "mode1";
  char name[] = "Mode Z1";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/select/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/select/frisquet/%s/state",
  "command_topic": "homeassistant/select/frisquet/%s/set",
  "options": ["Auto", "Confort", "Réduit", "Hors gel", "Inconnu"],
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
})", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterModeZ2() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "mode2";
  char name[] = "Mode Z2";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/select/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/select/frisquet/%s/state",
  "command_topic": "homeassistant/select/frisquet/%s/set",
  "options": ["Auto", "Confort", "Réduit", "Hors gel", "Inconnu"],
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterModeZ3() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "mode3";
  char name[] = "Mode Z3";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/select/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/select/frisquet/%s/state",
  "command_topic": "homeassistant/select/frisquet/%s/set",
  "options": ["Auto", "Confort", "Réduit", "Hors gel", "Inconnu"],
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterConsoGazChauffage() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "consogaz-ch";
  char name[] = "Consommation Gaz chauffage";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "kWh",
  "device_class": "energy",
  "state_class": "total_increasing",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterConsoGazECS() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "consogaz-ecs";
  char name[] = "Consommation Gaz ECS";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/sensor/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/sensor/frisquet/%s/state",
  "unit_of_measurement": "kWh",
  "device_class": "energy",
  "state_class": "total_increasing",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneHorsGelZ1() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneHorsGel1";
  char name[] = "Temperature consigne Hors Gel Z1";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneReduitZ1() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneReduit1";
  char name[] = "Temperature consigne réduit Z1";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneConfortZ1() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneConfort1";
  char name[] = "Temperature consigne Confort Z1";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneHorsGelZ2() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneHorsGel2";
  char name[] = "Temperature consigne Hors Gel Z2";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneReduitZ2() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneReduit2";
  char name[] = "Temperature consigne réduit Z2";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneConfortZ2() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneConfort2";
  char name[] = "Temperature consigne Confort Z2";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneHorsGelZ3() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneHorsGel3";
  char name[] = "Temperature consigne Hors Gel Z3";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneReduitZ3() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneReduit3";
  char name[] = "Temperature consigne réduit Z3";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterTemperatureConsigneConfortZ3() {
  char configTopic[70];
  char configPayload[500];
  char id[] = "tempConsigneConfort3";
  char name[] = "Temperature consigne Confort Z3";

  snprintf(configTopic, sizeof(configTopic), "homeassistant/number/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/number/frisquet/%s/state",
  "command_topic": "homeassistant/number/frisquet/%s/set",
  "min": 6,
  "max": 30,
  "mode": "box",
  "step": 0.5,
  "unit_of_measurement": "°C",
  "device_class": "temperature",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
}

void MQTT::connecterBoostZ1() {
  char configTopic[60];
  char configPayload[400];
  char id[] = "boost1";
  char name[] = "Boost Z1";
  snprintf(configTopic, sizeof(configTopic), "homeassistant/switch/frisquet/%s/config", id);
  snprintf(configPayload, sizeof(configPayload), R"(
{
  "uniq_id": "frisquet_%s",
  "name": "%s",
  "state_topic": "homeassistant/switch/frisquet/%s/state",
  "command_topic": "homeassistant/switch/frisquet/%s/set",
  "payload_on": "ON",
  "payload_off": "OFF",
  "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
}
)", id, name, id, id);
  this->client.publish(configTopic, configPayload);
  //this->client.publish(BOOST_TOPIC, "OFF");
  //this->client.publish(BOOST, "OFF");
}


void MQTT::connectTopics() {
  if (this->client.connected()) {
    this->connecterTemperatureAmbianteZ1();
    this->connecterTemperatureConsigneZ1();
    this->connecterModeZ1();

    this->connecterTemperatureConsigneHorsGelZ1();
    this->connecterTemperatureConsigneReduitZ1();
    this->connecterTemperatureConsigneConfortZ1();

    if (USE_ZONE_2) {
      this->connecterTemperatureAmbianteZ2();
      this->connecterTemperatureConsigneZ2();
      this->connecterModeZ2();

      this->connecterTemperatureConsigneHorsGelZ2();
      this->connecterTemperatureConsigneReduitZ2();
      this->connecterTemperatureConsigneConfortZ2();
    }

    if (USE_ZONE_3) {
      this->connecterTemperatureAmbianteZ3();
      this->connecterTemperatureConsigneZ3();
      this->connecterModeZ3();

      this->connecterTemperatureConsigneHorsGelZ3();
      this->connecterTemperatureConsigneReduitZ3();
      this->connecterTemperatureConsigneConfortZ3();
    }

    this->connecterTemperatureExterieure();

    this->connecterTemperatureCDC();
    this->connecterTemperatureDepartZ1();

    this->connecterAssociationSondeExterieure();
    this->connecterAssociationConnect();

    this->connecterEffacerNVS();

    this->connecterConsoGazChauffage();
    
    this->connecterTemperatureECS();
    this->connecterConsoGazECS();

    this->connecterBoostZ1();
    this->connecterPayload();


    // Souscrire aux topics temp ambiante, consigne, ext et mode
    this->client.subscribe("homeassistant/select/frisquet/mode1/set");
    this->client.subscribe("homeassistant/switch/frisquet/asssonde/set");
    this->client.subscribe("homeassistant/switch/frisquet/assconnect/set");
    //this->client.subscribe("homeassistant/sensor/frisquet/tempAmbiante1/state");
    //this->client.subscribe("homeassistant/sensor/frisquet/tempExterieure/state");
    //this->client.subscribe("homeassistant/sensor/frisquet/tempConsigne1/state");

    this->client.subscribe("homeassistant/number/frisquet/tempConsigneHorsGel1/set");
    this->client.subscribe("homeassistant/number/frisquet/tempConsigneReduit1/set");
    this->client.subscribe("homeassistant/number/frisquet/tempConsigneConfort1/set");
    
    /*this->client.subscribe("homeassistant/select/frisquet/mode1/state");
    this->client.subscribe("homeassistant/number/frisquet/tempConsigneHorsGel1/state");
    this->client.subscribe("homeassistant/number/frisquet/tempConsigneReduit1/state");
    this->client.subscribe("homeassistant/number/frisquet/tempConsigneConfort1/state");*/

    if(USE_ZONE_2) {
      //this->client.subscribe("homeassistant/sensor/frisquet/tempAmbiante2/state");
      //this->client.subscribe("homeassistant/sensor/frisquet/tempConsigne2/state");
      this->client.subscribe("homeassistant/select/frisquet/mode2/set");

      this->client.subscribe("homeassistant/number/frisquet/tempConsigneHorsGel2/set");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneReduit2/set");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneConfort2/set");

      /*this->client.subscribe("homeassistant/select/frisquet/mode2/state");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneHorsGel2/state");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneReduit2/state");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneConfort2/state");*/
    }

    if(USE_ZONE_3) {
      //this->client.subscribe("homeassistant/sensor/frisquet/tempAmbiante3/state");
      //this->client.subscribe("homeassistant/sensor/frisquet/tempConsigne3/state");
      this->client.subscribe("homeassistant/select/frisquet/mode3/set");

      this->client.subscribe("homeassistant/number/frisquet/tempConsigneHorsGel3/set");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneReduit3/set");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneConfort3/set");

      /*this->client.subscribe("homeassistant/select/frisquet/mode3/state");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneHorsGel3/state");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneReduit3/state");
      this->client.subscribe("homeassistant/number/frisquet/tempConsigneConfort3/state");*/
    }

    #if USE_SONDE_EXTERIEURE
      this->client.subscribe("homeassistant/sensor/frisquet/tempExterieure/set");
    #endif

    this->client.subscribe("homeassistant/switch/frisquet/boost1/set");
    this->client.subscribe("homeassistant/status");
  }
}


void MQTT::setCallback(MQTT_CALLBACK_SIGNATURE) {
    this->client.setCallback(callback);
}

boolean MQTT::publish(const char *topic, const char *payload, bool retain) {
    return this->client.publish(topic, payload, retain);
}

boolean MQTT::publish(const char *topic, float value, bool retain) {
  char payload[10];
  snprintf(payload, sizeof(payload), "%.2f", value);
  return this->publish(topic, payload);
}

boolean MQTT::loop() {
  if(! this->connected()) {
    if(this->connectToMQTT()) {
      this->connectTopics();
    }
  }

  return this->client.loop();
}


/*
//****************************************************************************
// Déclare un thermostat HA via MQTT
void connectToClimate()
{
  char configPayload[2048];
  snprintf(configPayload, sizeof(configPayload), R"(
    {
      "uniq_id": "frisquet_thermostat1",
      "name": "Thermostat1",
      "modes": ["heat"],
      "mode_command_topic": "homeassistant/climate/frisquet/mode/set",
      "mode_state_topic": "homeassistant/climate/frisquet/mode/state",
      "temperature_unit": "C",
      "precision": 0.1,
      "temp_step": 0.5,
      "min_temp": 5,
      "max_temp": 30,
      "current_temperature_topic": "homeassistant/sensor/frisquet/tempAmbiante1/state",
      "temperature_command_topic": "homeassistant/sensor/frisquet/tempConsigne1/set",
      "temperature_state_topic": "homeassistant/sensor/frisquet/tempConsigne1/state",
      "preset_mode_command_topic": "homeassistant/select/frisquet/mode/set",
      "preset_mode_state_topic": "homeassistant/select/frisquet/mode/state",
      "preset_modes": ["Auto", "Confort", "Réduit", "Hors gel"],
      "device":{"ids":["Frisquet_MQTT"],"mf":"HA Community","name":"Frisquet MQTT","mdl":"ESP32 Heltec"}
    })");
  publishMessage("homeassistant/climate/frisquet/Thermostat1/config", configPayload);
}
  */
