#include "Satellite.h"
#include <math.h>
#include "../Buffer.h"

void Satellite::loadConfig() {
    getPreferences().begin((String("satCfgZ") + String(getNumeroZone())).c_str(), false);
    setIdAssociation(getPreferences().getUChar("idAssociation", 0xFF));
    
    setMode((MODE)getPreferences().getUChar("mode", MODE::CONFORT_PERMANENT));
    setTemperatureAmbiante(getPreferences().getFloat("tempAmbiante", NAN));
    setTemperatureConsigne(getPreferences().getFloat("tempConsigne", NAN));
    setTemperatureConfort(getPreferences().getFloat("tempConfort", NAN));
    setTemperatureReduit(getPreferences().getFloat("tempReduit", NAN));
    setTemperatureHorsGel(getPreferences().getFloat("tempHorsGel", NAN));
    setTemperatureBoost(getPreferences().getFloat("tempBoost", 2));

    getPreferences().end();
}

void Satellite::saveConfig() {   
    getPreferences().begin((String("satCfgZ") + String(getNumeroZone())).c_str(), false);
    getPreferences().putUChar("idAssociation", getIdAssociation());

    getPreferences().putUChar("mode", getMode());
    getPreferences().putFloat("tempAmbiante", getTemperatureAmbiante());
    getPreferences().putFloat("tempConsigne", getTemperatureConsigne());
    getPreferences().putFloat("tempConfort", getTemperatureConfort());
    getPreferences().putFloat("tempReduit", getTemperatureReduit());
    getPreferences().putFloat("tempHorsGel", getTemperatureHorsGel());
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
    _mqttEntities.temperatureBoost.id = "temperatureBoostSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureBoost.name = "Température Boost Satellite Z" + String(getNumeroZone());
    _mqttEntities.temperatureBoost.component = "number";
    _mqttEntities.temperatureBoost.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureBoost"}), 0, true);
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
            saveConfig();
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
                saveConfig();
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
                if(confortActif()) {
                    setTemperatureConfort(temperature);
                }
                if(reduitActif()) {
                    setTemperatureReduit(temperature);
                }
                if(horsGelActif()) {
                    setTemperatureHorsGel(temperature);
                }
                mqtt().publishState(_mqttEntities.temperatureConsigne, getTemperatureConsigne());
                saveConfig();
                envoyerConsigne();
            }
        });
    }
    mqtt().registerEntity(*device, _mqttEntities.temperatureConsigne, true);

    _mqttEntities.temperatureConfort.id = "temperatureConfortSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureConfort.name = "Température confort Z" + String(getNumeroZone());
    _mqttEntities.temperatureConfort.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureConfort"}), 0, true);
    _mqttEntities.temperatureConfort.set("device_class", "temperature");
    _mqttEntities.temperatureConfort.set("state_class", "measurement");
    _mqttEntities.temperatureConfort.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureConfort.component = "number";
    _mqttEntities.temperatureConfort.set("min", "5");
    _mqttEntities.temperatureConfort.set("max", "30");
    _mqttEntities.temperatureConfort.set("mode", "box");
    _mqttEntities.temperatureConfort.set("step", "0.5");
    _mqttEntities.temperatureConfort.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureConfort", "set"}), 0, true);
    mqtt().onCommand(_mqttEntities.temperatureConfort, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[SATELLITE Z%d] Modification de la température confort à %0.2f.", getNumeroZone(), temperature);
            setTemperatureConfort(temperature);
            mqtt().publishState(_mqttEntities.temperatureConfort, getTemperatureConfort());
            saveConfig();
            if(confortActif()) {
                envoyerConsigne();
            }
        }
    });
    mqtt().registerEntity(*device, _mqttEntities.temperatureConfort, true);

    _mqttEntities.temperatureReduit.id = "temperatureReduitSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureReduit.name = "Température reduit Z" + String(getNumeroZone());
    _mqttEntities.temperatureReduit.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureReduit"}), 0, true);
    _mqttEntities.temperatureReduit.set("device_class", "temperature");
    _mqttEntities.temperatureReduit.set("state_class", "measurement");
    _mqttEntities.temperatureReduit.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureReduit.component = "number";
    _mqttEntities.temperatureReduit.set("min", "5");
    _mqttEntities.temperatureReduit.set("max", "30");
    _mqttEntities.temperatureReduit.set("mode", "box");
    _mqttEntities.temperatureReduit.set("step", "0.5");
    _mqttEntities.temperatureReduit.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureReduit", "set"}), 0, true);
    mqtt().onCommand(_mqttEntities.temperatureReduit, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[SATELLITE Z%d] Modification de la température réduit à %0.2f.", getNumeroZone(), temperature);
            setTemperatureReduit(temperature);
            mqtt().publishState(_mqttEntities.temperatureReduit, getTemperatureReduit());
            saveConfig();
            if(reduitActif()) {
                envoyerConsigne();
            }
        }
    });
    mqtt().registerEntity(*device, _mqttEntities.temperatureReduit, true);

    _mqttEntities.temperatureHorsGel.id = "temperatureHorsGelSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureHorsGel.name = "Température Hors-Gel Z" + String(getNumeroZone());
    _mqttEntities.temperatureHorsGel.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureHorsGel"}), 0, true);
    _mqttEntities.temperatureHorsGel.set("device_class", "temperature");
    _mqttEntities.temperatureHorsGel.set("state_class", "measurement");
    _mqttEntities.temperatureHorsGel.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureHorsGel.component = "number";
    _mqttEntities.temperatureHorsGel.set("min", "5");
    _mqttEntities.temperatureHorsGel.set("max", "30");
    _mqttEntities.temperatureHorsGel.set("mode", "box");
    _mqttEntities.temperatureHorsGel.set("step", "0.5");
    _mqttEntities.temperatureHorsGel.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic, "satellite", "z" + String(getNumeroZone()),"temperatureHorsGel", "set"}), 0, true);
    mqtt().onCommand(_mqttEntities.temperatureHorsGel, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[SATELLITE Z%d] Modification de la température hors-gel à %0.2f.", getNumeroZone(), temperature);
            setTemperatureHorsGel(temperature);
            mqtt().publishState(_mqttEntities.temperatureHorsGel, getTemperatureHorsGel());
            saveConfig();
            if(horsGelActif()) {
                envoyerConsigne();
            }
        }
    });
    mqtt().registerEntity(*device, _mqttEntities.temperatureHorsGel, true);
    

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
        envoyerConsigne();
    });

    // SELECT: Mode zone
    _mqttEntities.mode.id = "modeChauffageSatZ" + String(getNumeroZone());
    _mqttEntities.mode.name = "Mode Chauffage Sat Z" + String(getNumeroZone());
    _mqttEntities.mode.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode"}), 0, true);
    _mqttEntities.mode.set("icon", "mdi:tune-variant");
    _mqttEntities.mode.component = "select";
    _mqttEntities.mode.setRaw("options", R"(["Confort","Réduit", "Hors Gel"])");
    _mqttEntities.mode.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode", "set"}), 0, true);
    mqtt().onCommand(_mqttEntities.mode, [&](const String& payload){
        setMode(payload);
        info("[SATELLITE Z%d] Modification du mode %s.", getNumeroZone(), getNomMode());
        mqtt().publishState(_mqttEntities.mode, getNomMode());
        envoyerConsigne();
    });
    mqtt().registerEntity(*device, _mqttEntities.mode, true);

    // SWITCH : Actif
    _mqttEntities.actif.id = "actifSatZ" + String(getNumeroZone());
    _mqttEntities.actif.name = "Satellite Z" + String(getNumeroZone()) + " actif";
    _mqttEntities.actif.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"actif"}), 0, true);
    _mqttEntities.actif.set("icon", "mdi:tune-variant");
    _mqttEntities.actif.component = "switch";
    _mqttEntities.actif.set("payload_on", "heat");
    _mqttEntities.actif.set("state_on", "heat");
    _mqttEntities.actif.set("payload_off", "off");
    _mqttEntities.actif.set("state_off", "off");
    _mqttEntities.actif.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"actif", "set"}), 0, true);
    mqtt().onCommand(_mqttEntities.actif, [&](const String& payload){
        info("[SATELLITE Z%d] Modification de l'état actif : %s.", getNumeroZone(), payload);
        setActif(payload.equalsIgnoreCase("heat"));
        mqtt().publishState(_mqttEntities.actif, payload);
        envoyerConsigne();
    });
    mqtt().registerEntity(*device, _mqttEntities.actif, false);

    
    // THERMOSTAT
    _mqttEntities.thermostat.id = "thermostatSatZ" + String(getNumeroZone());
    _mqttEntities.thermostat.name = "Thermostat Z" + String(getNumeroZone());
    _mqttEntities.thermostat.component = "climate";
    _mqttEntities.thermostat.set("icon", "mdi:tune-variant");
    _mqttEntities.thermostat.setRaw("modes", R"(["heat", "off"])");
    _mqttEntities.thermostat.set("temperature_unit", "C");
    _mqttEntities.thermostat.set("precision", 0.1);
    _mqttEntities.thermostat.set("temp_step", 0.5);
    _mqttEntities.thermostat.set("min_temp", 5);
    _mqttEntities.thermostat.set("max_temp", 30);
    _mqttEntities.thermostat.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"actif"}));
    _mqttEntities.thermostat.set("mode_state_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"actif"}));
    _mqttEntities.thermostat.set("mode_command_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"actif", "set"}));
    _mqttEntities.thermostat.set("preset_mode_command_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode","set"}));
    _mqttEntities.thermostat.set("preset_mode_state_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"mode"}));
    _mqttEntities.thermostat.set("current_temperature_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureAmbiante"}));
    _mqttEntities.thermostat.set("temperature_command_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureConsigne", "set"}));
    _mqttEntities.thermostat.set("temperature_state_topic", MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"temperatureConsigne"}));
    _mqttEntities.thermostat.setRaw("preset_modes", R"(["Confort","Réduit", "Hors Gel"])");
    mqtt().registerEntity(*device, _mqttEntities.thermostat, true);
}

void Satellite::loop() {
    
    static bool firstLoop = true;
    if(firstLoop) {
        firstLoop = false;
        publishMqtt();
    }

    uint32_t now = millis();

    if(! estAssocie() || !_modeVirtuel || !actif()) {
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

    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("actifSatZ" + String(getNumeroZone())), actif() ? "heat": "off");

    if(!isnan(getTemperatureAmbiante())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("tempAmbSatZ" + String(getNumeroZone())), getTemperatureAmbiante());
    }
    if(!isnan(getTemperatureConsigne())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("tempConsSatZ" + String(getNumeroZone())), getTemperatureConsigne());
    }
    if(!isnan(getTemperatureBoost())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureBoostSatZ" + String(getNumeroZone())), getTemperatureBoost());
    }
    if(!isnan(getTemperatureConfort())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConfortSatZ" + String(getNumeroZone())), getTemperatureConfort());
    }
    if(!isnan(getTemperatureReduit())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureReduitSatZ" + String(getNumeroZone())), getTemperatureReduit());
    }
    if(!isnan(getTemperatureHorsGel())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureHorsGelSatZ" + String(getNumeroZone())), getTemperatureHorsGel());
    }
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("boostSatZ" + String(getNumeroZone())), boostActif() ? "ON" : "OFF");
    if(getMode() != MODE::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeChauffageSatZ" + String(getNumeroZone())), getNomMode().c_str());
    }
}

bool Satellite::confortActif() {
    return _mode == MODE::CONFORT_AUTO || _mode == MODE::CONFORT_PERMANENT || _mode == MODE::CONFORT_DEROGATION;
}
bool Satellite::reduitActif() {
    return _mode == MODE::REDUIT_AUTO || _mode == MODE::REDUIT_PERMANENT || _mode == MODE::REDUIT_DEROGATION;
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
    if(! estAssocie() || getNumeroZone() == 0 || !actif()) {
        return false;
    }

    if(confortActif()) {
        setTemperatureConsigne(_temperatureConfort);
    } else if (reduitActif()) {
        setTemperatureConsigne(_temperatureReduit);
    } else if (horsGelActif()) {
        setTemperatureConsigne(_temperatureHorsGel);
    }

    if(isnan(getTemperatureAmbiante()) || isnan(getTemperatureConsigne()) || getMode() == MODE::INCONNU) {
        warning("[SATELLITE Z%d] Aucune température de consigne ou ambiance.", getNumeroZone());
        return false;
    }

    if(boostActif()) {
        setTemperatureConsigne(getTemperatureConsigne() + getTemperatureBoost());
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

    debug("Envoi consigne FAKE, amb : %0.2f, cons : %0.2f", getTemperatureAmbiante(), getTemperatureConsigne());
    return true;
    
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
            0xA02F + (0x0005 * (getNumeroZone() - 1)),
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

                setTemperatureAmbiante(donneesSatellite->temperatureAmbiante.toFloat());

                if(!actif()) {
                    setMode((MODE)donneesSatellite->mode);
                    setTemperatureConsigne(donneesSatellite->temperatureConsigne.toFloat());
                    saveConfig();
                    publishMqtt();
                    return true;
                }

                setMode(MODE::CONFORT_PERMANENT);

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
    } else {
        if(header->idAssociation != getIdAssociation() || header->isAck()) {
            return false;
        }

        radio().sendAnswer(getId(), header->idExpediteur, header->idAssociation, header->idMessage, header->idReception, header->type, {}, 0);
        return true;
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
            return "Confort";
        case MODE::REDUIT_AUTO:
            return "Réduit";
        case MODE::CONFORT_PERMANENT:
            return "Confort";
        case MODE::REDUIT_PERMANENT:
            return "Réduit";
        case MODE::CONFORT_DEROGATION:
            return "Confort";
        case MODE::REDUIT_DEROGATION:
            return "Réduit";
        case MODE::HORS_GEL:
            return "Hors Gel";
    }

    return "Inconnu";
}

void Satellite::setTemperatureConfort(float temperature) {
    temperature = round(temperature * 2) / 2.0f;
    this->_temperatureConfort = std::min(30.0f, std::max(5.0f, temperature));
}
void Satellite::setTemperatureReduit(float temperature) {
    temperature = round(temperature * 2) / 2.0f;
    this->_temperatureReduit = std::min(30.0f, std::max(5.0f, temperature));
}
void Satellite::setTemperatureHorsGel(float temperature) {
    temperature = round(temperature * 2) / 2.0f;
    this->_temperatureHorsGel = std::min(30.0f, std::max(5.0f, temperature));
}
void Satellite::setTemperatureAmbiante(float temperature) {
    this->_temperatureAmbiante = temperature;
}
void Satellite::setTemperatureConsigne(float temperature) {
    this->_temperatureConsigne = std::min(30.0f, std::max(5.0f, temperature));
}
void Satellite::setTemperatureBoost(float temperature) {
    temperature = round(temperature * 2) / 2.0f;
    //this->_temperatureBoost = std::min(30.0f, std::max(5.0f, temperature));
    this->_temperatureBoost = std::min(5.0f, std::max(0.5f, temperature));
}