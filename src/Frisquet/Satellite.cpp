#include "Satellite.h"
#include <math.h>
#include "../Buffer.h"

void Satellite::loadConfig() {
    getPreferences().begin((String("satCfgZ") + String(getNumeroZone())).c_str(), false);
    setIdAssociation(getPreferences().getUChar("idAssociation", 0xFF));
    
    setMode((MODE)getPreferences().getUChar("mode", MODE::CONFORT_PERMANENT));
    setTemperatureAmbiante(getPreferences().getFloat("tempAmbiante", NAN));
    setTemperatureConsigne(getPreferences().getFloat("tempConsigne", NAN));
    setTemperatureBoost(getPreferences().getFloat("tempBoost", 2));

    getPreferences().end();
}

void Satellite::saveConfig() {   
    getPreferences().begin((String("satCfgZ") + String(getNumeroZone())).c_str(), false);
    getPreferences().putUChar("idAssociation", getIdAssociation());

    getPreferences().putUChar("mode", getMode());
    getPreferences().putFloat("tempAmbiante", getTemperatureAmbiante());
    getPreferences().putFloat("tempConsigne", getTemperatureConsigne());
    getPreferences().putFloat("tempBoost", getTemperatureBoost());

    getPreferences().end();
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
    _mqttEntities.temperatureBoost.set("max", "5");
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

    _mqttEntities.temperatureAmbiante.id = "tempAmbSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiante.name = "Température Ambiante Satellite Z" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiante.component = "sensor";
    _mqttEntities.temperatureAmbiante.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureAmbiante"}), 0, true);
    _mqttEntities.temperatureAmbiante.set("device_class", "temperature");
    _mqttEntities.temperatureAmbiante.set("state_class", "measurement");
    _mqttEntities.temperatureAmbiante.set("unit_of_measurement", "°C");
    if(_modeVirtuel) {
        _mqttEntities.temperatureAmbiante.component = "number";
        _mqttEntities.temperatureAmbiante.set("min", "0");
        _mqttEntities.temperatureAmbiante.set("max", "50");
        _mqttEntities.temperatureAmbiante.set("mode", "box");
        _mqttEntities.temperatureAmbiante.set("step", "0.1");
        _mqttEntities.temperatureAmbiante.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureAmbiante", "set"}), 0, true);
        mqtt().onCommand(_mqttEntities.temperatureAmbiante, [&](const String& payload) {
            float temperature = payload.toFloat();
            if(!isnan(temperature)) {
                info("[SATELLITE Z%d] Modification de la température ambiante à %0.2f.", getNumeroZone(), temperature);
                setTemperatureAmbiante(temperature);
                mqtt().publishState(_mqttEntities.temperatureAmbiante, getTemperatureAmbiante());
            }
        });
    }
    mqtt().registerEntity(*device, _mqttEntities.temperatureAmbiante, true);


    _mqttEntities.temperatureConsigne.id = "tempConsSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureConsigne.name = "Température consigne Z" + String(getNumeroZone());
    _mqttEntities.temperatureConsigne.component = "sensor";
    _mqttEntities.temperatureConsigne.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureConsigne"}), 0, true);
    _mqttEntities.temperatureConsigne.set("device_class", "temperature");
    _mqttEntities.temperatureConsigne.set("state_class", "measurement");
    _mqttEntities.temperatureConsigne.set("unit_of_measurement", "°C");
    if(_modeVirtuel) {
        _mqttEntities.temperatureConsigne.component = "number";
        _mqttEntities.temperatureConsigne.set("min", "5");
        _mqttEntities.temperatureConsigne.set("max", "30");
        _mqttEntities.temperatureConsigne.set("mode", "box");
        _mqttEntities.temperatureConsigne.set("step", "0.5");
        _mqttEntities.temperatureConsigne.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureConsigne", "set"}), 0, true);
        mqtt().onCommand(_mqttEntities.temperatureConsigne, [&](const String& payload) {
            float temperature = payload.toFloat();
            if(!isnan(temperature)) {
                info("[SATELLITE Z%d] Modification de la température consigne à %0.2f.", getNumeroZone(), temperature);
                setTemperatureConsigne(temperature);
                mqtt().publishState(_mqttEntities.temperatureConsigne, getTemperatureConsigne());
                envoyerConsigne();
            }
        });
    }
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
    _mqttEntities.mode.set("icon", "mdi:tune-variant");
    /*if(_modeVirtuel) {
        _mqttEntities.mode.component = "select";
        _mqttEntities.mode.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode","set"}), 0, true);
        _mqttEntities.mode.setRaw("options", R"(["Confort","Réduit"])");
        _mqttEntities.mode.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode", "set"}), 0, true);
    }*/
    mqtt().registerEntity(*device, _mqttEntities.mode, true);
    
}

void Satellite::loop() {
    
    static bool firstLoop = true;
    if(firstLoop) {
        firstLoop = false;
        publishMqtt();
    }

    uint32_t now = millis();

    if(! estAssocie() || !_modeVirtuel) {
        return;
    }

    if (now - _lastEnvoiConsigne >= 600000 || _lastEnvoiConsigne == 0) { // 10 minutes
        info("[SATELLITE Z%d] Envoi de la consigne.", getNumeroZone());
        setMode(MODE::CONFORT_PERMANENT);
        if(envoyerConsigne()) {
            incrementIdMessage(3);
            _lastEnvoiConsigne = now;
            publishMqtt();
        } else {
            error("[SATELLITE Z%d] Echec de l'envoi.", getNumeroZone());
            _lastEnvoiConsigne = now <= 60000 ? 1 : now - 60000;
        }
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

    if(isnan(getTemperatureAmbiante()) || isnan(getTemperatureConsigne()) || getMode() == MODE::INCONNU) {
        return false;
    }
    
    struct donneesSatellite_t {
        temperature16 temperatureAmbiante; 
        temperature16 temperatureConsigne;
        uint8_t i1 = 0x00; 
        uint8_t mode = 0x00; // 0x01 Confort, 0x02 Reduit, etc.
        fword modeOptions = (uint16_t)0x0000;
    } payload;

    struct donneesZones_t {
        FrisquetRadio::RadioTrameHeader header;
        uint8_t longueur;
        temperature16 temperatureExterieure;
        byte i1[2];
        uint8_t date[6];  // format reçu "YY MM DD hh mm ss"
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
    } donneesZones;
    
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

    
    size_t length = 0;
    uint16_t err;

    uint8_t retry = 0;
    do {
        length = sizeof(donneesZones_t);
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
            (byte*)&donneesZones,
            length
        );


        Date date = donneesZones.date;
        setDate(date);
        
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
    if(!_modeVirtuel) {
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

                if(! boostActif()) {
                    delay(300);

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
                        uint8_t date[6];  // format reçu "YY MM DD hh mm ss"
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
                    }

                    Date date = donneesZones->date;
                    setDate(date);
                }

                setIdAssociation(header->idAssociation);
                setIdMessage(header->idMessage);
                setMode((MODE)donneesSatellite->mode);
                setTemperatureAmbiante(donneesSatellite->temperatureAmbiante.toFloat());
                setTemperatureConsigne(donneesSatellite->temperatureConsigne.toFloat());
                
                if(! boostActif() || isnan(getTemperatureBoost())) {
                    saveConfig();
                    publishMqtt();
                    return true;
                }

                info("[SATELLITE Z%d] Écrasement de données.", getNumeroZone());

                incrementIdMessage(3);

                if(!this->envoyerConsigne()) {
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