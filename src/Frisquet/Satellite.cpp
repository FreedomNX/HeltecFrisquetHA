#include "Satellite.h"
#include <math.h>
#include "../Buffer.h"

void Satellite::loadConfig() {
    /*getPreferences().begin("sondeExtCfg", true);
    setIdAssociation(getPreferences().getUChar("idAssociation", 0xFF));
    getPreferences().end();*/
}

void Satellite::saveConfig() {   
    /*getPreferences().begin("sondeExtCfg", false);
    getPreferences().putUChar("idAssociation", getIdAssociation());
    getPreferences().end();*/
}


void Satellite::begin() {
    loadConfig();

    // Initialisation MQTT
  info("[SATELLITE Z%d] Initialisation des entités.", getNumeroZone());

    // Device commun
  MqttDevice* device = mqtt().getDevice("heltecFrisquet");
  
    // Entités

    // SENSOR: Température boost
    _mqttEntities.temperatureBoost.id = "tempBoostSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureBoost.name = "Température Boost Satellite Z" + String(getNumeroZone());
    _mqttEntities.temperatureBoost.component = "number";
    _mqttEntities.temperatureBoost.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureBoost"}), 0, true);
    _mqttEntities.temperatureBoost.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureBoost", "set"}), 0, true);
    _mqttEntities.temperatureBoost.set("device_class", "temperature");
    _mqttEntities.temperatureBoost.set("state_class", "measurement");
    _mqttEntities.temperatureBoost.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureBoost.set("min", "0");
    _mqttEntities.temperatureBoost.set("max", "30");
    _mqttEntities.temperatureBoost.set("mode", "box");
    _mqttEntities.temperatureBoost.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.temperatureBoost, true);
    mqtt().onCommand(_mqttEntities.temperatureBoost, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[SATELLITE Z%d] Modification de la température de boost à %0.2f.", getNumeroZone(), temperature);
            setTemperatureBoost(temperature);
            mqtt().publishState(_mqttEntities.temperatureBoost, getTemperatureBoost());
            if(estAssocie()) {
                envoyerConsigne();
            }
        }
    });

    _mqttEntities.temperatureAmbiance.id = "tempAmbSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiance.name = "Température Ambiante Satellite Z" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiance.component = "sensor";
    _mqttEntities.temperatureAmbiance.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureAmbiance"}), 0, true);
    _mqttEntities.temperatureAmbiance.set("device_class", "temperature");
    _mqttEntities.temperatureAmbiance.set("state_class", "measurement");
    _mqttEntities.temperatureAmbiance.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.temperatureAmbiance, true);


    _mqttEntities.temperatureConsigne.id = "tempConsSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureConsigne.name = "Température consigne Z" + String(getNumeroZone());
    _mqttEntities.temperatureConsigne.component = "sensor";
    _mqttEntities.temperatureConsigne.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureConsigne"}), 0, true);
    _mqttEntities.temperatureConsigne.set("device_class", "temperature");
    _mqttEntities.temperatureConsigne.set("state_class", "measurement");
    _mqttEntities.temperatureConsigne.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.temperatureConsigne, true);

    // SWITCH: Activation Boost
    _mqttEntities.boost.id = "boostSatZ" + String(getNumeroZone());
    _mqttEntities.boost.name = "Boost Satellite Z" + String(getNumeroZone());
    _mqttEntities.boost.component = "switch";
    _mqttEntities.boost.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"boost"}), 0, true);
    _mqttEntities.boost.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"boost", "set"}), 0, true);
    _mqttEntities.boost.set("icon", "mdi:tune-variant");
    _mqttEntities.boost.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.boost, true);
    mqtt().onCommand(_mqttEntities.boost, [&](const String& payload){
        payload.equalsIgnoreCase("ON") ? activerBoost() : desactiverBoost();
        mqtt().publishState(_mqttEntities.boost, boostActif() ? "ON" : "OFF");
        info("[SATELLITE Z%d] Modification du boost %s.", getNumeroZone(), boostActif() ? "ON" : "OFF");
        if(estAssocie()) {
            envoyerConsigne();
        }
    });

    // SELECT: Mode zone
    _mqttEntities.mode.id = "modeChauffageSatZ" + String(getNumeroZone());
    _mqttEntities.mode.name = "Mode Chauffage Sat Z" + String(getNumeroZone());
    _mqttEntities.mode.component = "sensor";
    _mqttEntities.mode.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode"}), 0, true);
    //_mqttEntities.modeZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"mode","set"}), 0, true);
    _mqttEntities.mode.set("icon", "mdi:tune-variant");
    //_mqttEntities.mode.set("entity_category", "config");
    //_mqttEntities.modeZ1.setRaw("options", R"(["Hors Gel","Réduit","Confort","Auto"])");
    mqtt().registerEntity(*device, _mqttEntities.mode, true);
   /* mqtt().onCommand(_mqttEntities.mode, [&](const String& payload){
        setMode(payload);
    });*/
}

void Satellite::loop() {
    
    static bool firstLoop = true;
    if(firstLoop) {
        firstLoop = false;
        publishMqtt();
    }

    uint32_t now = millis();

    if(estAssocie()) {
    }
}

void Satellite::publishMqtt() {
    if(!isnan(getTemperatureAmbiante())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("tempAmbSatZ" + String(getNumeroZone())), getTemperatureAmbiante());
    }
    if(!isnan(getTemperatureConsigne())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("tempConsSatZ" + String(getNumeroZone())), getTemperatureConsigne());
    }
    if(!isnan(getTemperatureBoost())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("tempBoostSatZ" + String(getNumeroZone())), getTemperatureBoost());
    }
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("boostSatZ" + String(getNumeroZone())), boostActif() ? "ON" : "OFF");
    if(getMode() != MODE::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeChauffageSatZ" + String(getNumeroZone())), getNomMode().c_str());
    }
}

bool Satellite::confortActif() {
    return (_mode & 0b00000001) != 0;
}
bool Satellite::reduitActif() {
    return (_mode & 0b00000001) == 0;
}
bool Satellite::horsGelActif() {
    return _mode == MODE::HORS_GEL;
}
bool Satellite::derogationActive() {
    return (_mode & 0b00000010) == 1;
}
bool Satellite::autoActif() {
    return (_mode & 0b00000100) == 1 || derogationActive();
}

bool Satellite::envoyerConsigne() {
    if(! estAssocie()) {
        return false;
    }
    
    struct donneesSatellite_t {
        temperature16 temperatureAmbiante; 
        temperature16 temperatureConsigne;
        uint8_t i1 = 0x00; 
        uint8_t mode = 0x00; // 0x01 Confort, 0x02 Reduit, etc.
        uint8_t i2[2] = {0};
    } payload;
    
    payload.temperatureAmbiante = getTemperatureAmbiante();
    payload.temperatureConsigne = getTemperatureConsigne();
    payload.mode = getMode();

    if(boostActif()) {
        payload.temperatureConsigne = getTemperatureConsigne() + getTemperatureBoost();
        if(getMode() == MODE::REDUIT_AUTO) {
            payload.mode = MODE::CONFORT_DEROGATION;
        } else if(getMode() == MODE::REDUIT_DEROGATION) {
            payload.mode = MODE::CONFORT_AUTO;
        } else if(getMode() == MODE::REDUIT_PERMANENT) {
            payload.mode = MODE::CONFORT_PERMANENT;
        } else if(getMode() == MODE::HORS_GEL) {
            return false;
        }
    }

    
    byte buff[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    size_t length = 0;
    uint16_t err;

    incrementIdMessage(3);

    uint8_t retry = 0;
    do {
        err = this->radio().sendInit(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01, 
            0xA029,
            0x0015,
            0xA02F,
            0x0004,
            (byte*)&payload,
            sizeof(payload),
            buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(30);
            continue;
        }
        
        return true;
    } while(retry++ < 1);

    return false;
}

bool Satellite::onReceive(byte* donnees, size_t length) { 
    ReadBuffer readBuffer(donnees, length);
    FrisquetRadio::RadioTrameHeader* header = (FrisquetRadio::RadioTrameHeader*) readBuffer.getBytes(sizeof(FrisquetRadio::RadioTrameHeader));

    //info("[SATELLITE Z] Interception envoi consigne.", header->type);
    if(_modeEcrasement) {
        if(length == 23 && header->type == FrisquetRadio::MessageType::INIT && header->idExpediteur == this->getId() && header->idMessage != getIdMessage()) { // Récéption en écoute
            FrisquetRadio::RadioTrameInit* requete = (FrisquetRadio::RadioTrameInit*) readBuffer.getBytes(sizeof(FrisquetRadio::RadioTrameInit));
            if(requete->adresseMemoireEcriture.toUInt16() == 0xA02F && requete->tailleMemoireEcriture.toUInt16() == 0x0004) { // Envoi consigne

                struct donneesSatellite_t {
                    temperature16 temperatureAmbiante; 
                    temperature16 temperatureConsigne;
                    uint8_t i1 = 0x00; 
                    uint8_t mode = 0x00; // 0x01 Confort, 0x02 Reduit, etc.
                    uint8_t i2[2] = {0};
                }* donneesSatellite = (donneesSatellite_t*) readBuffer.getBytes(sizeof(*donneesSatellite));
                
                //info("[SATELLITE Z%d] Interception envoi consigne.", getNumeroZone());

                uint8_t retry = 0;
                uint16_t err;
                
                size_t lengthRx = 0;
                byte buffZones[RADIOLIB_SX126X_MAX_PACKET_LENGTH];

               /* delay(1000);

                err = radio().receiveExpected(
                    ID_CHAUDIERE,
                    this->getId(),
                    header->idAssociation, 
                    header->idMessage,
                    header->idReception|0x80,
                    header->type,
                    (byte*) buffZones,
                    lengthRx,
                    15, true
                );
                
                if(err != RADIOLIB_ERR_NONE) {
                    return false;
                }

                readBuffer = ReadBuffer(buffZones, lengthRx);

                struct donneesZones_t {
                    FrisquetRadio::RadioTrameHeader header;
                    uint8_t longueur;
                    temperature16 temperatureExterieure;
                    byte i1[2];
                    uint8_t dateheure[6];  // format reçu "YY MM DD hh mm ss"
                    byte modeChaudiere; // mode chaudière à valider
                    uint8_t jourSemaine; // format wday
                    temperature16 temperatureAmbianteZ1;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                    temperature16 temperatureConsigneZ1;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                    byte i2 = 0x00;
                    uint8_t modeZ1 = 0x00;                       // 0x05 auto - 0x06 confort - 0x07 reduit - 0x08 hors gel
                    byte i3[4] = {0x00, 0xC6, 0x00, 0xC6};
                    temperature16 temperatureAmbianteZ2;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                    temperature16 temperatureConsigneZ2;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                    byte i4 = 0x00;
                    uint8_t modeZ2 = 0x00;                       // 0x05 auto - 0x06 confort - 0x07 reduit - 0x08 hors gel
                    byte i5[4] = {0x00, 0x00, 0x00, 0x00};
                    temperature16 temperatureAmbianteZ3;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                    temperature16 temperatureConsigneZ3;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                    byte i6 = 0x00;
                    uint8_t modeZ3 = 0x00;                       // 0x05 auto - 0x06 confort - 0x07 reduit - 0x08 hors gel
                    byte i7[4] = {0x00, 0x00, 0x00, 0x00};
                }* donneesZones = (donneesZones_t*)readBuffer.getBytes(sizeof(donneesZones_t));

                if(lengthRx < sizeof(donneesZones_t)) {
                    return false;
                }*/

                setIdAssociation(header->idAssociation);
                setIdMessage(header->idMessage);
                setMode((MODE)donneesSatellite->mode);
                setTemperatureAmbiante(donneesSatellite->temperatureAmbiante.toFloat());
                setTemperatureConsigne(donneesSatellite->temperatureConsigne.toFloat());

                debug("[SATELLITE] Boost actif : %s.", boostActif() ? "Oui" : "Non");
                
                if(! boostActif() || isnan(getTemperatureBoost())) {
                    saveConfig();
                    publishMqtt();
                    return true;
                }

                info("[SATELLITE Z%d] Écrasement de données.", getNumeroZone());

                /*donneesSatellite->temperatureConsigne = (getTemperatureConsigne() + getTemperatureBoost());
                if(donneesSatellite->mode == MODE::REDUIT_AUTO) {
                    donneesSatellite->mode = MODE::CONFORT_DEROGATION;
                } else if(donneesSatellite->mode == MODE::REDUIT_DEROGATION) {
                    donneesSatellite->mode = MODE::CONFORT_AUTO;
                } else if(donneesSatellite->mode == MODE::REDUIT_PERMANENT) {
                    donneesSatellite->mode = MODE::CONFORT_PERMANENT;
                } else if(donneesSatellite->mode == MODE::HORS_GEL) {
                    return false;
                }


                incrementIdMessage(3);

                retry = 0;
                lengthRx = 0;
                do {
                    err = this->radio().sendInit(
                        this->getId(), 
                        ID_CHAUDIERE, 
                        getIdAssociation(),
                        incrementIdMessage(),
                        0x01, 
                        0xA029,
                        0x0015,
                        0xA02F,
                        0x0004,
                        (byte*)donneesSatellite,
                        sizeof(donneesSatellite_t),
                        buffZones,
                        lengthRx
                    );

                    if(err != RADIOLIB_ERR_NONE) {
                        delay(30);
                        continue;
                    }
                    
                    info("[SATELLITE Z%d] Écrasement réussie.", getNumeroZone());

                    saveConfig();
                    publishMqtt();
                    return true;
                } while(retry++ < 5);

                error("[SATELLITE Z%d] Échec de l'écrasement.", getNumeroZone());
                */

                if(this->envoyerConsigne()) {
                    error("[SATELLITE Z%d] Échec de l'écrasement.", getNumeroZone());
                } else {
                    info("[SATELLITE Z%d] Écrasement réussie.", getNumeroZone());
                }

                saveConfig();
                publishMqtt();
                return true;
            }
        }
    } 

    return false;
}

String Satellite::getNomMode() {

    0b00000000; // Reduit
    0b00000001; // Confort
    0b00000010; // Réduit dérog
    0b00000011; // Confort dérog
    0b00000100; // Auto réduit
    0b00000101; // Auto confort
    0b00010000; // Hors gel

    switch(this->getMode()) {
        case MODE::CONFORT_AUTO:
            return "Auto - Confort";
        case MODE::REDUIT_AUTO:
            return "Auto - Réduit";
        case MODE::CONFORT_PERMANENT:
            return "Confort";
        case MODE::REDUIT_PERMANENT:
            return "Réduit";
        case MODE::CONFORT_DEROGATION:
            return "Confort - Dérog";
        case MODE::REDUIT_DEROGATION:
            return "Réduit - Dérog";
    }

    return "Inconnu";
}