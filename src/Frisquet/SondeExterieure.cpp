#include "SondeExterieure.h"
#include <math.h>

void SondeExterieure::loadConfig() {
    bool checkMigration = false;
    getPreferences().begin("sondeExtCfg", false);
    
    if(! getPreferences().isKey("idAssociation")) {
        checkMigration = true;
    }

    setIdAssociation(getPreferences().getUChar("idAssociation", 0xFF));
    getPreferences().end();
    delay(100);

    // Migration
    if(checkMigration && getPreferences().begin("net-conf", false)) {
        if(getPreferences().isKey("son_id")) {
            setIdAssociation(getPreferences().getUChar("son_id", 0xFF));
        }
        getPreferences().end();
        checkMigration = false;
    }
}

void SondeExterieure::saveConfig() {   
    getPreferences().begin("sondeExtCfg", false);
    getPreferences().putUChar("idAssociation", getIdAssociation());
    getPreferences().end();
}

void SondeExterieure::setTemperatureExterieure(float temperatureExterieure) {
    _temperatureExterieure = std::min(std::max(-30.0f, (round(temperatureExterieure * 10.0f)/10.0f)), 80.0f);
}

float SondeExterieure::getTemperatureExterieure() {
    return _temperatureExterieure;
}

bool SondeExterieure::envoyerTemperatureExterieure() {
    if(! estAssocie() || isnan(_temperatureExterieure)) {
        return false;
    }

    struct {
        FrisquetRadio::RadioTrameHeader header;
        uint8_t longueur;
        byte date[6] = {0};
        uint8_t i1;
        uint8_t jour;
    } donnees;

    size_t length = 0;
    uint16_t err;
    
    uint8_t retry = 0;
    do {
        length = sizeof(donnees);
        err = this->radio().sendInit(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01, 
            0x9c54,
            0x0004,
            0xa029,
            0x0001,
            temperature16(_temperatureExterieure).bytes,
            sizeof(temperature16),
            (byte*)&donnees,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }

        Date date = donnees.date;
        setDate(date);
        
        return true;
    } while(retry++ < 1);

    return false;
}


void SondeExterieure::begin() {

    loadConfig();

    // Initialisation MQTT
  info("[SONDE EXTERIEURE][MQTT] Initialisation des entités.");

    // Device commun
  MqttDevice* device = mqtt().getDevice("heltecFrisquet");

  // Entités

  // SWITCH: Association Sonde Extérieure
    _mqttEntities.associationSondeExterieure.id = "associationSondeExterieure";
    _mqttEntities.associationSondeExterieure.name = "Association Sonde Extérieure";
    _mqttEntities.associationSondeExterieure.component = "switch";
    _mqttEntities.associationSondeExterieure.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"sondeExterieure","association"}), 0, true);
    _mqttEntities.associationSondeExterieure.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"sondeExterieure","association","set"}), 0, true);
    _mqttEntities.associationSondeExterieure.set("icon", "mdi:tune-variant");
    _mqttEntities.associationSondeExterieure.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.associationSondeExterieure, true);
    mqtt().onCommand(_mqttEntities.associationSondeExterieure, [&](const String& payload){
        NetworkID networkId;
        uint8_t idAssociation;
        if(associer(networkId, idAssociation)) {
            setIdAssociation(idAssociation);
            radio().setNetworkID(networkId);
            getConfig().setNetworkID(networkId);
            getConfig().save();
            saveConfig();
        }
    });


  // SENSOR: Température extérieure
  _mqttEntities.tempExterieure.id = "temperatureExterieureSonde";
  _mqttEntities.tempExterieure.name = "Température extérieure Sonde";
  _mqttEntities.tempExterieure.component = "number";
  _mqttEntities.tempExterieure.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "sondeExterieure", "temperatureExterieure"}), 0, true);
  _mqttEntities.tempExterieure.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"sondeExterieure","temperatureExterieure","set"}), 0, true);
  _mqttEntities.tempExterieure.set("device_class", "temperature");
  _mqttEntities.tempExterieure.set("state_class", "measurement");
  _mqttEntities.tempExterieure.set("unit_of_measurement", "°C");
  _mqttEntities.tempExterieure.set("min", "-30");
  _mqttEntities.tempExterieure.set("max", "80");
  _mqttEntities.tempExterieure.set("mode", "box");
  _mqttEntities.tempExterieure.set("step", "0.1");
  mqtt().registerEntity(*device, _mqttEntities.tempExterieure, true);
  mqtt().onCommand(_mqttEntities.tempExterieure, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[SONDE EXTERIEURE] Modification manuelle de la température extérieure à %0.2f.", temperature);
            setTemperatureExterieure(payload.toFloat());
            mqtt().publishState(_mqttEntities.tempExterieure, getTemperatureExterieure());
        }
    });
}

void SondeExterieure::loop() {
   uint32_t now = millis();

    if(estAssocie()) {
        if (now - _lastEnvoiTemperatureExterieure >= 600000 || _lastEnvoiTemperatureExterieure == 0) { // 10 minutes
            info("[SONDE EXTERIEURE] Envoi de la température extérieure.");
            // Récupération de la température si DS18B20 activé.
            if(_ds18b20 != nullptr && _ds18b20->isReady()) {
                float temperature = NAN;
                if(_ds18b20->getTemperature(temperature)) {
                    info("[DS18B20] Température : %.2f", temperature);
                    setTemperatureExterieure(temperature);
                }
            }
            
            if(!isnan(getTemperatureExterieure())) {
                if(envoyerTemperatureExterieure()) {
                    publishMqtt();
                    _lastEnvoiTemperatureExterieure = now;
                } else {
                    error("[SONDE EXTERIEURE] Echec de l'envoi de la température extérieure.");
                    _lastEnvoiTemperatureExterieure = now <= 60000 ? 1 : now - 60000;; // Essai dans 1 minute
                    
                }
            } else {
                warning("[SONDE EXTERIEURE] Aucune température disponible.");
                _lastEnvoiTemperatureExterieure = now;
            }
        }
    }
}

void SondeExterieure::publishMqtt() {
    if(!isnan(getTemperatureExterieure())) {
        mqtt().publishState(_mqttEntities.tempExterieure, getTemperatureExterieure());
    }
}