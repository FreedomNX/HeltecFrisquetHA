#include "Zone.h"


void Zone::loadConfig() {
    bool checkMigration = false;

    _preferences.begin("zoneCfg" + getNumeroZone(), false);

    setMode((Zone::MODE_ZONE)_preferences.getUChar("mode"));
    setModeOptions(_preferences.getUChar("modeOpts"));
    setTemperatureConfort(_preferences.getFloat("tempConfort"));
    setTemperatureReduit(_preferences.getFloat("tempReduit"));
    setTemperatureHorsGel(_preferences.getFloat("tempHorsGel"));
    setTemperatureBoost(_preferences.getFloat("tempBoost", 2));
    setTemperatureAmbiante(_preferences.getFloat("tempAmbiante", NAN));
    setTemperatureConsigne(_preferences.getFloat("tempConsigne", NAN));

    Zone::Programmation programmation;
    _preferences.getBytes("prog", &programmation, sizeof(Zone::Programmation));
    setProgrammation(programmation);

    _preferences.end();
}

void Zone::saveConfig() {   
    _preferences.begin("zoneCfg" + getNumeroZone(), false);

    _preferences.putUChar("mode", getMode());
    _preferences.putUChar("modeOpts", getModeOptions());
    _preferences.putFloat("tempConfort", getTemperatureConfort());
    _preferences.putFloat("tempReduit", getTemperatureReduit());
    _preferences.putFloat("tempHorsGel", getTemperatureHorsGel());
    _preferences.putFloat("tempBoost", getTemperatureBoost());
    _preferences.putBytes("prog", &getProgrammation(), sizeof(getProgrammation()));
    _preferences.putFloat("tempAmbiante", getTemperatureAmbiante());
    _preferences.putFloat("tempConsigne", getTemperatureConsigne());

    _preferences.end();
}


void Zone::begin() {
    
    loadConfig();

    // Device commun
    MqttDevice* device = mqtt().getDevice("heltecFrisquet");

    // SELECT: Mode zone
    _mqttEntities.mode.id = "modeChauffageZ" + String(getNumeroZone());
    _mqttEntities.mode.name = "Mode Chauffage Z" + String(getNumeroZone());
    _mqttEntities.mode.component = "select";
    _mqttEntities.mode.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"mode"}), 0, true);
    _mqttEntities.mode.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"mode","set"}), 0, true);
    _mqttEntities.mode.set("icon", "mdi:tune-variant");
    _mqttEntities.mode.set("entity_category", "config");
    _mqttEntities.mode.setRaw("options", R"(["Hors Gel", "Réduit","Confort", "Auto", "Boost"])");
    mqtt().registerEntity(*device, _mqttEntities.mode, true);
    mqtt().onCommand(_mqttEntities.mode, [&](const String& payload){
        setMode(payload);
        if(getSource() == SOURCE::SATELLITE_VIRTUEL) {
            if(confortActif()) {
                setTemperatureConsigne(getTemperatureConfort());
            } else if(reduitActif()) {
                setTemperatureConsigne(getTemperatureReduit());
            } else if(horsGelActif()) {
                setTemperatureConsigne(getTemperatureHorsGel());
            }

            info("[ZONE Z%d] Changement du mode : %d %s.", getNumeroZone(), getMode(), getNomMode());
            mqtt().publishState(_mqttEntities.temperatureConsigne, getTemperatureConsigne());
            mqtt().publishState(_mqttEntities.mode, getNomMode());
            refreshLastChange();
            saveConfig();
        }
    });

    // SENSOR: Température ambiante
    _mqttEntities.temperatureAmbiante.id = "temperatureAmbianteZ" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiante.name = "Température ambiante Z" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiante.component = "sensor";
    _mqttEntities.temperatureAmbiante.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureAmbiante"}), 0, true);
    _mqttEntities.temperatureAmbiante.set("device_class", "temperature");
    _mqttEntities.temperatureAmbiante.set("state_class", "measurement");
    _mqttEntities.temperatureAmbiante.set("unit_of_measurement", "°C");
    if(getSource() == SOURCE::SATELLITE_VIRTUEL) {
        _mqttEntities.temperatureAmbiante.component = "number";
        _mqttEntities.temperatureAmbiante.set("min", "0");
        _mqttEntities.temperatureAmbiante.set("max", "50");
        _mqttEntities.temperatureAmbiante.set("mode", "box");
        _mqttEntities.temperatureAmbiante.set("step", "0.1");
        _mqttEntities.temperatureAmbiante.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureAmbiante", "set"}), 0, true);
        mqtt().onCommand(_mqttEntities.temperatureAmbiante, [&](const String& payload) {
            float temperature = payload.toFloat();
            if(!isnan(temperature)) {
                info("[ZONE Z%d] Modification de la température ambiante à %0.2f.", getNumeroZone(), temperature);
                setTemperatureAmbiante(temperature);
                mqtt().publishState(_mqttEntities.temperatureAmbiante, getTemperatureAmbiante());
                saveConfig();
            }
        });
    }
    mqtt().registerEntity(*device, _mqttEntities.temperatureAmbiante, true);

    // SENSOR: Température consigne
    _mqttEntities.temperatureConsigne.id = "temperatureConsigneZ" + String(getNumeroZone());
    _mqttEntities.temperatureConsigne.name = "Consigne Z" + String(getNumeroZone());
    _mqttEntities.temperatureConsigne.component = "sensor";
    _mqttEntities.temperatureConsigne.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureConsigne"}), 0, true);
    _mqttEntities.temperatureConsigne.set("device_class", "temperature");
    _mqttEntities.temperatureConsigne.set("state_class", "measurement");
    _mqttEntities.temperatureConsigne.set("unit_of_measurement", "°C");
    if(getSource() == SOURCE::SATELLITE_VIRTUEL) {
        /*_mqttEntities.temperatureConsigne.component = "number";
        _mqttEntities.temperatureConsigne.set("min", "5");
        _mqttEntities.temperatureConsigne.set("max", "30");
        _mqttEntities.temperatureConsigne.set("mode", "box");
        _mqttEntities.temperatureConsigne.set("step", "0.5");*/
        _mqttEntities.temperatureConsigne.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureConsigne", "set"}), 0, true);
        mqtt().onCommand(_mqttEntities.temperatureConsigne, [&](const String& payload) {
            float temperature = payload.toFloat();
            if(!isnan(temperature)) {
                info("[ZONE Z%d] Modification de la température consigne à %0.2f.", getNumeroZone(), temperature);
                setTemperatureConsigne(temperature);
                mqtt().publishState(_mqttEntities.temperatureConsigne, getTemperatureConsigne());
                refreshLastChange();
                saveConfig();
            }
        });
    }
    mqtt().registerEntity(*device, _mqttEntities.temperatureConsigne, true);

    // SENSOR: Température confort
    _mqttEntities.temperatureConfort.id = "temperatureConfortZ" + String(getNumeroZone());
    _mqttEntities.temperatureConfort.name = "Température Confort Z" + String(getNumeroZone());
    _mqttEntities.temperatureConfort.component = "number";
    _mqttEntities.temperatureConfort.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureConfort"}), 0, true);
    _mqttEntities.temperatureConfort.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureConfort", "set"}), 0, true);
    _mqttEntities.temperatureConfort.set("device_class", "temperature");
    _mqttEntities.temperatureConfort.set("state_class", "measurement");
    _mqttEntities.temperatureConfort.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureConfort.set("min", "5");
    _mqttEntities.temperatureConfort.set("max", "30");
    _mqttEntities.temperatureConfort.set("mode", "box");
    _mqttEntities.temperatureConfort.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.temperatureConfort, true);
    mqtt().onCommand(_mqttEntities.temperatureConfort, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[ZONE %d] Modification de la température confort à %0.2f.", getNumeroZone(), temperature);
            setTemperatureConfort(temperature);
            mqtt().publishState(_mqttEntities.temperatureConfort, temperature);
            refreshLastChange();
            saveConfig();
        }
    });

    // SENSOR: Température réduite
    _mqttEntities.temperatureReduit.id = "temperatureReduitZ" + String(getNumeroZone());
    _mqttEntities.temperatureReduit.name = "Température Réduit Z" + String(getNumeroZone());
    _mqttEntities.temperatureReduit.component = "number";
    _mqttEntities.temperatureReduit.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureReduit"}), 0, true);
    _mqttEntities.temperatureReduit.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureReduit", "set"}), 0, true);
    _mqttEntities.temperatureReduit.set("device_class", "temperature");
    _mqttEntities.temperatureReduit.set("state_class", "measurement");
    _mqttEntities.temperatureReduit.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureReduit.set("min", "5");
    _mqttEntities.temperatureReduit.set("max", "30");
    _mqttEntities.temperatureReduit.set("mode", "box");
    _mqttEntities.temperatureReduit.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.temperatureReduit, true);
    mqtt().onCommand(_mqttEntities.temperatureReduit, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[ZONE %d] Modification de la température réduit à %0.2f.", getNumeroZone(), temperature);
            setTemperatureReduit(temperature);
            mqtt().publishState(_mqttEntities.temperatureReduit, temperature);
            refreshLastChange();
            saveConfig();
        }
    });

    // SENSOR: Température hors-gel
    _mqttEntities.temperatureHorsGel.id = "temperatureHorsGelZ" + String(getNumeroZone());
    _mqttEntities.temperatureHorsGel.name = "Température Hors-Gel Z" + String(getNumeroZone());
    _mqttEntities.temperatureHorsGel.component = "number";
    _mqttEntities.temperatureHorsGel.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureHorsGel"}), 0, true);
    _mqttEntities.temperatureHorsGel.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureHorsGel", "set"}), 0, true);
    _mqttEntities.temperatureHorsGel.set("device_class", "temperature");
    _mqttEntities.temperatureHorsGel.set("state_class", "measurement");
    _mqttEntities.temperatureHorsGel.set("unit_of_measurement", "°C");
    _mqttEntities.temperatureHorsGel.set("min", "5");
    _mqttEntities.temperatureHorsGel.set("max", "30");
    _mqttEntities.temperatureHorsGel.set("mode", "box");
    _mqttEntities.temperatureHorsGel.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.temperatureHorsGel, true);
    mqtt().onCommand(_mqttEntities.temperatureHorsGel, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[ZONE %d] Modification de la température hors-gel à %0.2f.", getNumeroZone(), temperature);
            setTemperatureHorsGel(temperature);
            mqtt().publishState(_mqttEntities.temperatureHorsGel, temperature);
            refreshLastChange();
            saveConfig();
        }
    });

    // SENSOR: Température départ
    _mqttEntities.temperatureDepart.id = "temperatureDepartZ" + String(getNumeroZone());
    _mqttEntities.temperatureDepart.name = "Température Départ Z" + String(getNumeroZone());
    _mqttEntities.temperatureDepart.component = "sensor";
    _mqttEntities.temperatureDepart.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"temperatureDepart"}), 0, true);
    _mqttEntities.temperatureDepart.set("device_class", "temperature");
    _mqttEntities.temperatureDepart.set("state_class", "measurement");
    _mqttEntities.temperatureDepart.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.temperatureDepart, true);

    // SENSOR: Température boost
    _mqttEntities.temperatureBoost.id = "temperatureBoostZ" + String(getNumeroZone());
    _mqttEntities.temperatureBoost.name = "Température Boost Z" + String(getNumeroZone());
    _mqttEntities.temperatureBoost.component = "number";
    _mqttEntities.temperatureBoost.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z"+ String(getNumeroZone()),"temperatureBoost"}), 0, true);
    _mqttEntities.temperatureBoost.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z"+ String(getNumeroZone()),"temperatureBoost", "set"}), 0, true);
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
            info("[ZONE %d] Modification de la température de boost à %0.2f.", getNumeroZone(), temperature);
            setTemperatureBoost(temperature);
            mqtt().publishState(_mqttEntities.temperatureBoost, temperature);
            saveConfig();
            if(boostActif()) {
                refreshLastChange();
            }
        }
    });

    // SWITCH: Activation Boost
    _mqttEntities.boost.id = "boostZ" + String(getNumeroZone());
    _mqttEntities.boost.name = "Boost Z" + String(getNumeroZone());
    _mqttEntities.boost.component = "switch";
    _mqttEntities.boost.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"boost"}), 0, true);
    _mqttEntities.boost.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z" + String(getNumeroZone()),"boost", "set"}), 0, true);
    _mqttEntities.boost.set("icon", "mdi:tune-variant");
    mqtt().registerEntity(*device, _mqttEntities.boost, true);
    mqtt().onCommand(_mqttEntities.boost, [&](const String& payload){
        if(payload.equalsIgnoreCase("ON")) { 
            info("[ZONE %d] Activation du boost", getNumeroZone());
            activerBoost();
        } else {
            info("[ZONE %d] Désactivation du boost", getNumeroZone());
            desactiverBoost();
        }
        saveConfig();
        refreshLastChange();
        mqtt().publishState(_mqttEntities.boost, payload);
        mqtt().publishState(_mqttEntities.mode, getNomMode());
    });


    // THERMOSTAT
    _mqttEntities.thermostat.id = "thermostatZ" + String(getNumeroZone());
    _mqttEntities.thermostat.name = "Thermostat Z" + String(getNumeroZone());
    _mqttEntities.thermostat.component = "climate";
    _mqttEntities.thermostat.set("icon", "mdi:tune-variant");
    _mqttEntities.thermostat.setRaw("modes", R"(["auto"])");
    _mqttEntities.thermostat.set("temperature_unit", "C");
    _mqttEntities.thermostat.set("precision", 0.1);
    _mqttEntities.thermostat.set("temp_step", 0.5);
    _mqttEntities.thermostat.set("min_temp", 5);
    _mqttEntities.thermostat.set("max_temp", 30);
    _mqttEntities.thermostat.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"thermostat"}));
    _mqttEntities.thermostat.set("mode_state_topic", MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"thermostat"}));
    _mqttEntities.thermostat.set("preset_mode_command_topic", MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"mode","set"}));
    _mqttEntities.thermostat.set("preset_mode_state_topic", MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"mode"}));
    _mqttEntities.thermostat.set("current_temperature_topic", MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureAmbiante"}));
    _mqttEntities.thermostat.set("temperature_command_topic", MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureConsigne", "set"}));
    _mqttEntities.thermostat.set("temperature_state_topic", MqttManager::compose({device->baseTopic, "z" + String(getNumeroZone()),"temperatureConsigne"}));
    _mqttEntities.thermostat.setRaw("preset_modes", R"(["Confort","Réduit", "Hors Gel", "Auto", "Boost"])");
    mqtt().registerEntity(*device, _mqttEntities.thermostat, true);
}


void Zone::setTemperatureConfort(float temperature) {
    if(isnan(temperature)) {
        this->_temperatureConfort = NAN;
        return;
    }
    temperature = round(temperature * 2) / 2.0f;
    this->_temperatureConfort = std::min(30.0f, std::max(5.0f, temperature));
}
void Zone::setTemperatureReduit(float temperature) {
    if(isnan(temperature)) {
        this->_temperatureReduit = NAN;
        return;
    }
    temperature = round(temperature * 2) / 2.0f;
    this->_temperatureReduit = std::min(30.0f, std::max(5.0f, temperature));
}
void Zone::setTemperatureHorsGel(float temperature) {
    if(isnan(temperature)) {
        this->_temperatureHorsGel = NAN;
        return;
    }
    temperature = round(temperature * 2) / 2.0f;
    this->_temperatureHorsGel = std::min(30.0f, std::max(5.0f, temperature));
}
void Zone::setTemperatureAmbiante(float temperature) {
    if(isnan(temperature)) {
        this->_temperatureAmbiante = NAN;
        return;
    }
    this->_temperatureAmbiante = temperature;
}
void Zone::setTemperatureConsigne(float temperature) {
    if(isnan(temperature)) {
        this->_temperatureConsigne = NAN;
        return;
    }
    this->_temperatureConsigne = std::min(30.0f, std::max(5.0f, temperature));
}
void Zone::setTemperatureBoost(float temperature) {
    if(isnan(temperature)) {
        this->_temperatureBoost = NAN;
        return;
    }
    temperature = round(temperature * 2) / 2.0f;
    //this->_temperatureBoost = std::min(30.0f, std::max(5.0f, temperature));
    this->_temperatureBoost = std::min(5.0f, std::max(0.5f, temperature));
}
void Zone::setTemperatureDepart(float temperature) {
    this->_temperatureDepart = temperature;
}

float Zone::getTemperatureConfort() {
    return this->_temperatureConfort;
}
float Zone::getTemperatureReduit() {
    return this->_temperatureReduit;
}
float Zone::getTemperatureHorsGel() {
    return this->_temperatureHorsGel;
}
float Zone::getTemperatureConsigne() {
    return this->_temperatureConsigne;
}
float Zone::getTemperatureAmbiante() {
    return this->_temperatureAmbiante;
}
float Zone::getTemperatureDepart() {
    return this->_temperatureDepart;
}
float Zone::getTemperatureBoost() {
    return this->_temperatureBoost;
}

Zone::MODE_ZONE Zone::getMode() {
    return (MODE_ZONE)this->_mode;
}

void Zone::setMode(MODE_ZONE mode, bool confort, bool derogation) {
    _mode = mode;
    if(_mode == MODE_ZONE::AUTO) {
        if(confort) _modeOptions |= 0b00000001;
        else _modeOptions &= ~0b00000001;

        if(derogation) _modeOptions |= 0b00000010;
        else _modeOptions &= ~0b00000010;
    }
}
void Zone::setMode(const String& mode, bool confort, bool derogation) {
  if (mode.equalsIgnoreCase("Auto")) {
    this->setMode(MODE_ZONE::AUTO, confort, derogation);
    this->desactiverBoost();
  } else if (mode.equalsIgnoreCase("Réduit")) {
    this->setMode(MODE_ZONE::REDUIT);
    this->desactiverBoost();
  } else if (mode.equalsIgnoreCase("Hors Gel")) {
    this->setMode(MODE_ZONE::HORS_GEL);
    this->desactiverBoost();
  } else if (mode.equalsIgnoreCase("Confort")) {
    this->setMode(MODE_ZONE::CONFORT);
    this->desactiverBoost();
  } else if (mode.equalsIgnoreCase("Boost")) {
    this->setMode(MODE_ZONE::CONFORT);
    this->activerBoost();
  }
}

String Zone::getNomMode() {
    switch(this->getMode()) {
        case MODE_ZONE::AUTO:
            return "Auto";
            break;
        case MODE_ZONE::CONFORT:
            return boostActif() ? "Boost" : "Confort";
            break;
        case MODE_ZONE::REDUIT:
            return "Réduit";
            break;
        case MODE_ZONE::HORS_GEL:
            return "Hors Gel";
            break;
    }

    return "Inconnu";
}

byte Zone::getModeOptions() {
    return this->_modeOptions;
}
void Zone::setModeOptions(byte modeOptions) {
    this->_modeOptions = modeOptions;
}

uint8_t Zone::getIdZone() {
    return this->_idZone;
}

void Zone::activerBoost() {
    _modeOptions |= 0b01000000;
}
void Zone::desactiverBoost() {
    _modeOptions &= ~0b01000000;
}
bool Zone::boostActif() {
    return (_modeOptions & 0b01000000) != 0;
}
uint8_t Zone::getNumeroZone() {
    switch(_idZone) {
            case ID_ZONE_1: 
                return 1;
            case ID_ZONE_2: 
                return 2;
            case ID_ZONE_3: 
                return 3;
        }
        
        return 0;
}
bool Zone::confortActif() {
    return 
        _mode == MODE_ZONE::CONFORT || 
        _mode == MODE_ZONE::AUTO && (_modeOptions &= 0b00000001 == 1);
}

bool Zone::reduitActif() {
    return 
        _mode == MODE_ZONE::REDUIT || 
        _mode == MODE_ZONE::AUTO && (_modeOptions &= 0b00000001 == 0);
}

bool Zone::horsGelActif() {
    return _mode == MODE_ZONE::HORS_GEL;
}

bool Zone::derogationActive() {
    return 
        _mode == MODE_ZONE::AUTO && (_modeOptions &= 0b00000010 == 1);
}


void Zone::publishMqtt() {
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("thermostatZ" + String(getNumeroZone())), "auto");
    if(!isnan(getTemperatureAmbiante())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureAmbianteZ" + String(getNumeroZone())), getTemperatureAmbiante());
    }
    if(!isnan(getTemperatureConsigne())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConsigneZ" + String(getNumeroZone())), getTemperatureConsigne());
    }
    if(!isnan(getTemperatureDepart())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureDepartZ" + String(getNumeroZone())), getTemperatureDepart());
    }
    if(!isnan(getTemperatureConfort())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConfortZ" + String(getNumeroZone())), getTemperatureConfort());
    }
    if(!isnan(getTemperatureReduit())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureReduitZ" + String(getNumeroZone())), getTemperatureReduit());
    }
    if(!isnan(getTemperatureHorsGel())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureHorsGelZ" + String(getNumeroZone())), getTemperatureHorsGel());
    }
    if(!isnan(getTemperatureBoost())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureBoostZ" + String(getNumeroZone())), getTemperatureBoost());
    }
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("boostZ" + String(getNumeroZone())), boostActif() ? "ON" : "OFF");
    if(getMode() != Zone::MODE_ZONE::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeChauffageZ" + String(getNumeroZone())), getNomMode().c_str());
    }
}