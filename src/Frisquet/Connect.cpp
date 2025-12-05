#include "Connect.h"
#include "../Buffer.h"
#include <math.h>

void Connect::loadConfig() {
    bool checkMigration = false;

    getPreferences().begin("connectCfg", true);

    if(! getPreferences().isKey("idAssociation")) {
        checkMigration = true;
    }

    setIdAssociation(getPreferences().getUChar("idAssociation", 0xFF));

    if(getPreferences().isKey("z1")) {
        _zone1.setMode((Zone::MODE_ZONE)getPreferences().getUChar("z1/mode"));
        _zone1.setModeOptions(getPreferences().getUChar("z1/modeOpts"));
        _zone1.setTemperatureConfort(getPreferences().getFloat("z1/tempConfort"));
        _zone1.setTemperatureReduit(getPreferences().getFloat("z1/tempReduit"));
        _zone1.setTemperatureHorsGel(getPreferences().getFloat("z1/tempHorsGel"));
        _zone1.setTemperatureBoost(getPreferences().getFloat("z1/tempBoost"));

        Zone::Programmation programmationZ1;
        getPreferences().getBytes("z1/prog", &programmationZ1, sizeof(Zone::Programmation));
        _zone1.setProgrammation(programmationZ1);
    } 
    
    if(getPreferences().isKey("z2")) {
        _zone2.setMode((Zone::MODE_ZONE)getPreferences().getUChar("z2/mode"));
        _zone2.setModeOptions(getPreferences().getUChar("z2/modeOpts"));
        _zone2.setTemperatureConfort(getPreferences().getFloat("z2/tempConfort"));
        _zone2.setTemperatureReduit(getPreferences().getFloat("z2/tempReduit"));
        _zone2.setTemperatureHorsGel(getPreferences().getFloat("z2/tempHorsGel"));
        _zone2.setTemperatureBoost(getPreferences().getFloat("z2/tempBoost"));

        Zone::Programmation programmationZ2;
        getPreferences().getBytes("z2/prog", &programmationZ2, sizeof(Zone::Programmation));
        _zone2.setProgrammation(programmationZ2);
    }

    if(getPreferences().isKey("z3")) {
        _zone3.setMode((Zone::MODE_ZONE)getPreferences().getUChar("z3/mode"));
        _zone3.setModeOptions(getPreferences().getUChar("z3/modeOpts"));
        _zone3.setTemperatureConfort(getPreferences().getFloat("z3/tempConfort"));
        _zone3.setTemperatureReduit(getPreferences().getFloat("z3/tempReduit"));
        _zone3.setTemperatureHorsGel(getPreferences().getFloat("z3/tempHorsGel"));
        _zone3.setTemperatureBoost(getPreferences().getFloat("z3/tempBoost"));

        Zone::Programmation programmationZ3;
        getPreferences().getBytes("z3/prog", &programmationZ3, sizeof(Zone::Programmation));
        _zone3.setProgrammation(programmationZ3);
    }

    getPreferences().end();

    // Migration
    if(checkMigration && getPreferences().begin("net-conf", false)) {
        if(getPreferences().isKey("con_id")) {
            setIdAssociation(getPreferences().getUChar("con_id", 0xFF));
        }
        getPreferences().end();
        checkMigration = false;
    }
}

void Connect::saveConfig() {   
    getPreferences().begin("connectCfg", false);

    getPreferences().putUChar("idAssociation", getIdAssociation());

    getPreferences().putBool("z1", true);
    getPreferences().putUChar("z1/mode", _zone1.getMode());
    getPreferences().putUChar("z1/modeOpts", _zone1.getModeOptions());
    getPreferences().putFloat("z1/tempConfort", _zone1.getTemperatureConfort());
    getPreferences().putFloat("z1/tempReduit", _zone1.getTemperatureReduit());
    getPreferences().putFloat("z1/tempHorsGel", _zone1.getTemperatureHorsGel());
    getPreferences().putFloat("z1/tempBoost", _zone1.getTemperatureBoost());
    getPreferences().putBytes("z1/prog", &_zone1.getProgrammation(), sizeof(_zone1.getProgrammation()));

    getPreferences().putBool("z2", true);
    getPreferences().putUChar("z2/mode", _zone2.getMode());
    getPreferences().putUChar("z2/modeOpts", _zone2.getModeOptions());
    getPreferences().putFloat("z2/tempConfort", _zone2.getTemperatureConfort());
    getPreferences().putFloat("z2/tempReduit", _zone2.getTemperatureReduit());
    getPreferences().putFloat("z2/tempHorsGel", _zone2.getTemperatureHorsGel());
    getPreferences().putFloat("z2/tempBoost", _zone2.getTemperatureBoost());
    getPreferences().putBytes("z2/prog", &_zone2.getProgrammation(), sizeof(_zone2.getProgrammation()));

    getPreferences().putBool("z3", true);
    getPreferences().putUChar("z3/mode", _zone3.getMode());
    getPreferences().putUChar("z3/modeOpts", _zone3.getModeOptions());
    getPreferences().putFloat("z3/tempConfort", _zone3.getTemperatureConfort());
    getPreferences().putFloat("z3/tempReduit", _zone3.getTemperatureReduit());
    getPreferences().putFloat("z3/tempHorsGel", _zone3.getTemperatureHorsGel());
    getPreferences().putFloat("z3/tempBoost", _zone3.getTemperatureBoost());
    getPreferences().putBytes("z3/prog", &_zone3.getProgrammation(), sizeof(_zone3.getProgrammation()));

    getPreferences().end();
}

bool Connect::envoyerZone(Zone& zone) {
    if(! estAssocie()) {
        return false;
    }
    
    struct {
        temperature8 temperatureConfort;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
        temperature8 temperatureReduit;     // Début 5°C -> 0 = 50 = 5°C - MAX Confort
        temperature8 temperatureHorsGel;     // Début 5°C -> 0 = 50 = 5°C - MAX Hors gel
        uint8_t mode = 0x00; 
        byte modeOptions = 0b00000100;
        /*
        Mode Option structure bits
            inconnu1: 1 bit,
            boost: 1 bit,
            inconnu2: 2 bits,
            inconnu3: 2 bits,
            derogation: 1 bit,
            confort: 1 bit
        */
        byte inconnu1 = 0x00; // Incrément boost ?
        //TODO Ajouter la programmation de la semaine
    } payload;
    
    if(zone.boostActif()) {
        payload.temperatureConfort = zone.getTemperatureConfort() + zone.getTemperatureBoost();
    } else {
        payload.temperatureConfort = zone.getTemperatureConfort();
    }
    payload.temperatureReduit = zone.getTemperatureReduit();
    payload.temperatureHorsGel = zone.getTemperatureHorsGel();
    payload.mode = zone.getMode();
    payload.modeOptions = zone.getModeOptions();
    
    byte buff[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    size_t length = 0;
    uint16_t err;

    uint8_t retry = 0;
    do {
        err = this->radio().sendInit(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            zone.getIdZone(), 
            0xA154,
            0x0018,
            0xa154,
            0x0003,
            (byte*)&payload,
            sizeof(payload),
            buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }
        
        return true;
    } while(retry++ < 1);

    return false;
}

bool Connect::recupererTemperatures() {
    if(! estAssocie()) {
        return false;
    }

    struct {
        FrisquetRadio::RadioTrameHeader header;
        uint8_t longueurDonnees;
        temperature16 temperatureECS;
        temperature16 temperatureCDC;
        temperature16 temperatureDepartZ1;
        temperature16 temperatureDepartZ2;
        temperature16 temperatureDepartZ3;
        temperature16 temperatureInconnue1;
        temperature16 temperatureInconnue2;
        temperature16 temperatureInconnue3;
        temperature16 temperatureInconnue4;
        temperature16 temperatureInconnue5;
        temperature8 temperatureInconnue6;
        temperature8 temperatureInconnue7;
        byte i1[1] = {0};
        byte modeECS;
        temperature16 temperatureECSInstant;
        byte i2[10] = {0};
        temperature16 temperatureAmbianteZ1;
        temperature16 temperatureAmbianteZ2;
        temperature16 temperatureAmbianteZ3;
        byte i3[6] = {0};
        temperature16 temperatureConsigneZ1;
        temperature16 temperatureConsigneZ2;
        temperature16 temperatureConsigneZ3;
        temperature16 temperatureExterieure;
    } buff;

    size_t length;
    uint16_t err;

    uint8_t retry = 0;
    do {
        length = sizeof(buff);
        err = this->radio().sendAsk(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01,
            0x79E0,
            0x001C,
            (byte*)&buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }

        getZone1().setTemperatureAmbiante(buff.temperatureAmbianteZ1.toFloat());
        getZone1().setTemperatureConsigne(buff.temperatureConsigneZ1.toFloat());
        getZone1().setTemperatureDepart(buff.temperatureDepartZ1.toFloat());

        getZone2().setTemperatureAmbiante(buff.temperatureAmbianteZ2.toFloat());
        getZone2().setTemperatureConsigne(buff.temperatureConsigneZ2.toFloat());
        getZone2().setTemperatureDepart(buff.temperatureDepartZ2.toFloat());

        getZone3().setTemperatureAmbiante(buff.temperatureAmbianteZ3.toFloat());
        getZone3().setTemperatureConsigne(buff.temperatureConsigneZ3.toFloat());
        getZone3().setTemperatureDepart(buff.temperatureDepartZ3.toFloat());

        setTemperatureExterieure(buff.temperatureExterieure.toFloat());
        setTemperatureECS(buff.temperatureECS.toFloat());
        setTemperatureCDC(buff.temperatureCDC.toFloat());
        
        return true;
    } while(retry++ < 1);

    return false;
}

bool Connect::recupererConsommation() {
    if(! estAssocie()) {
        return false;
    }

    struct {
        FrisquetRadio::RadioTrameHeader header;
        uint8_t longueurDonnees;
        byte i1[18] = {0};
        fword consommationECS;
        fword consommationChauffage;
        byte i2[34] = {0};
    } buff;
    

    size_t length;
    uint16_t err;

    uint8_t retry = 0;
    do {
        length = sizeof(buff);
        err = this->radio().sendAsk(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01,
            0x7A18,
            0x001C,
            (byte*)&buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }
        
        setConsommationChauffage(buff.consommationChauffage.toInt16());
        setConsommationECS(buff.consommationECS.toInt16());
        
        return true;
    } while(retry++ < 1);

    return false;
}

void Connect::setTemperatureExterieure(float temperature) {
    _temperatureExterieure = temperature;
}
void Connect::setTemperatureECS(float temperature) {
    _temperatureECS = temperature;
}
void Connect::setTemperatureCDC(float temperature) {
    _temperatureCDC = temperature;
}

float Connect::getTemperatureExterieure() {
    return _temperatureExterieure;
}
float Connect::getTemperatureECS() {
    return _temperatureECS;
}
float Connect::getTemperatureCDC() {
    return _temperatureCDC;
}

void Connect::setConsommationECS(int16_t consommation) {
    _consommationGazECS = consommation;
}
void Connect::setConsommationChauffage(int16_t consommation) {
    _consommationGazChauffage = consommation;
}
int16_t Connect::getConsommationChauffage() {
    return _consommationGazChauffage;
}
int16_t Connect::getConsommationECS() {
    return _consommationGazECS;
}

bool Connect::recupererModeECS() {
    if(! estAssocie()) {
        return false;
    }


    /*
    7E 80 AA 16 05 10 A0 F0 00 0D 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 11 // Eco horloge
    7E 80 AA 1A 05 10 A0 F0 00 0D 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 // Eco
    80 7E AA 03 05 10 A0 F0 00 0D 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 // Eco
    7E 80 AA 1D 05 10 A0 F0 00 0D 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 // max
    7E 80 AA 22 05 10 A0 F0 00 0D 1A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 19 // eco+
    */
    // Demande récupération courte : 80 7E AA 03 01 03 A0 FC 00 01
    struct {
        FrisquetRadio::RadioTrameHeader header;
        uint8_t longueurDonnees;
        byte i1;
        byte modeECS;
    } buff;
    

    size_t length;
    uint16_t err;

    uint8_t retry = 0;
    do {
        length = sizeof(buff);
        err = this->radio().sendAsk(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01,
            0xA0FC,
            0x0001,
            (byte*)&buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }
        
        setModeECS((MODE_ECS)buff.modeECS);

        return true;
    } while(retry++ < 1);

    return false;
}
Connect::MODE_ECS Connect::getModeECS() {
    return _modeECS;
}
bool Connect::setModeECS(MODE_ECS modeECS) {
    _modeECS = modeECS;
    return false;
}
bool Connect::envoyerModeECS() {
    if(! estAssocie()) {
        return false;
    }
    
    struct {
        uint8_t i1 = 0x00;
        uint8_t modeECS = 0x00; 
    } payload;
    
    payload.modeECS = getModeECS();
    
    byte buff[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    size_t length = 0;
    uint16_t err;

    uint8_t retry = 0;
    do {
        err = this->radio().sendInit(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01, 
            0xA0FC,
            0x0001,
            0xA0FC,
            0x0001,
            (byte*)&payload,
            sizeof(payload),
            buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }
        
        return true;
    } while(retry++ < 1);

    return false;
}

bool Connect::onReceive(byte* donnees, size_t length) {
    if(! estAssocie()) {
        return false;
    }

    ReadBuffer readBuffer(donnees, length);

    FrisquetRadio::RadioTrameHeader header;
    if(readBuffer.remainingLength() < sizeof(header)) { return false; }

    readBuffer.getBytes((byte*)&header, sizeof(header));

    if(header.type == FrisquetRadio::MessageType::INIT) {
        FrisquetRadio::RadioTrameInit requete;
        if(readBuffer.remainingLength() < sizeof(requete)) { return false; }
        readBuffer.getBytes((byte*)&requete, sizeof(requete));

        if(requete.adresseMemoireEcriture.toUInt16() == 0xA154 && requete.tailleMemoireEcriture.toUInt16() == 0x0018) { // Modification Zone
            info("[CONNECT] Récéption trame Zone");

            struct {
                temperature8 temperatureConfort;    // Début 5°C -> 0 = 50 = 5°C - MAX 30°C
                temperature8 temperatureReduit;     // Début 5°C -> 0 = 50 = 5°C - MAX Confort
                temperature8 temperatureHorsGel;     // Début 5°C -> 0 = 50 = 5°C - MAX Hors gel
                uint8_t mode = 0x00; 
                byte modeOptions = 0b00000100;
                byte inconnu1 = 0x00;
                byte dimanche[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                byte lundi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                byte mardi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                byte mercredi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                byte jeudi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                byte vendredi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                byte samedi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            } donneesZone;

            if(readBuffer.remainingLength() < sizeof(donneesZone)) { return false; }
            readBuffer.getBytes((byte*)&donneesZone, sizeof(donneesZone));


            getZone(header.idExpediteur).setMode((Zone::MODE_ZONE)donneesZone.mode);
            getZone(header.idExpediteur).setModeOptions(donneesZone.modeOptions);
            getZone(header.idExpediteur).setTemperatureReduit(donneesZone.temperatureReduit.toFloat());
            getZone(header.idExpediteur).setTemperatureHorsGel(donneesZone.temperatureHorsGel.toFloat());
            if(getZone(header.idExpediteur).boostActif()) {
                //getZone(header.idExpediteur).setTemperatureBoost(donneesZone.temperatureConfort.toFloat());
            } else {
                getZone(header.idExpediteur).setTemperatureConfort(donneesZone.temperatureConfort.toFloat());
            }

            saveConfig();

            uint8_t retry = 0;
            uint16_t err;
            
            do {
                err = radio().sendAnswer(
                    header.idDestinataire, 
                    header.idExpediteur, 
                    header.idAssociation, 
                    header.idMessage, 
                    header.idReception, 
                    header.type,
                    (byte*)&donneesZone,
                    sizeof(donneesZone)
                );
                if(err != RADIOLIB_ERR_NONE) {
                    delay(100);
                    continue;
                }

                publishMqtt();

                return true;
            } while (retry++ < 5);
        }
    }

    return false;
}


void Connect::begin() {

    loadConfig();

    // Initialisation MQTT
  info("[CONNECT][MQTT] Initialisation des entités.");

    // Device commun
  MqttDevice* device = mqtt().getDevice("heltecFrisquet");
  
  // Entités

  // SWITCH: Association Connect

    _mqttEntities.associationConnect.id = "associationConnect";
    _mqttEntities.associationConnect.name = "Association Connect";
    _mqttEntities.associationConnect.component = "switch";
    _mqttEntities.associationConnect.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"connect","association"}), 0, true);
    _mqttEntities.associationConnect.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"connect","association","set"}), 0, true);
    _mqttEntities.associationConnect.set("icon", "mdi:tune-variant");
    _mqttEntities.associationConnect.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.associationConnect, true);
    mqtt().onCommand(_mqttEntities.associationConnect, [&](const String& payload){
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
    
    // Zone 1

    // SELECT: Mode zone
    _mqttEntities.modeZ1.id = "modeChauffageZ1";
    _mqttEntities.modeZ1.name = "Mode Chauffage Z1";
    _mqttEntities.modeZ1.component = "select";
    _mqttEntities.modeZ1.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"z1","mode"}), 0, true);
    _mqttEntities.modeZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","mode","set"}), 0, true);
    _mqttEntities.modeZ1.set("icon", "mdi:tune-variant");
    _mqttEntities.modeZ1.set("entity_category", "config");
    _mqttEntities.modeZ1.setRaw("options", R"(["Hors Gel","Réduit","Confort","Auto"])");
    mqtt().registerEntity(*device, _mqttEntities.modeZ1, true);
    mqtt().onCommand(_mqttEntities.modeZ1, [&](const String& payload){
        getZone1().setMode(payload);
        _envoiZ1 = true;
        _lastEnvoiZone = 0;
    });

    // SENSOR: Température ambiante
    _mqttEntities.tempAmbianteZ1.id = "temperatureAmbianteZ1";
    _mqttEntities.tempAmbianteZ1.name = "Température ambiante Z1";
    _mqttEntities.tempAmbianteZ1.component = "sensor";
    _mqttEntities.tempAmbianteZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureAmbiante"}), 0, true);
    _mqttEntities.tempAmbianteZ1.set("device_class", "temperature");
    _mqttEntities.tempAmbianteZ1.set("state_class", "measurement");
    _mqttEntities.tempAmbianteZ1.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempAmbianteZ1, true);

    // SENSOR: Température consigne
    _mqttEntities.tempConsigneZ1.id = "temperatureConsigneZ1";
    _mqttEntities.tempConsigneZ1.name = "Consigne Z1";
    _mqttEntities.tempConsigneZ1.component = "sensor";
    _mqttEntities.tempConsigneZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureConsigne"}), 0, true);
    _mqttEntities.tempConsigneZ1.set("device_class", "temperature");
    _mqttEntities.tempConsigneZ1.set("state_class", "measurement");
    _mqttEntities.tempConsigneZ1.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempConsigneZ1, true);

    // SENSOR: Température confort
    _mqttEntities.tempConfortZ1.id = "temperatureConfortZ1";
    _mqttEntities.tempConfortZ1.name = "Température Confort Z1";
    _mqttEntities.tempConfortZ1.component = "number";
    _mqttEntities.tempConfortZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureConfort"}), 0, true);
    _mqttEntities.tempConfortZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureConfort", "set"}), 0, true);
    _mqttEntities.tempConfortZ1.set("device_class", "temperature");
    _mqttEntities.tempConfortZ1.set("state_class", "measurement");
    _mqttEntities.tempConfortZ1.set("unit_of_measurement", "°C");
    _mqttEntities.tempConfortZ1.set("min", "5");
    _mqttEntities.tempConfortZ1.set("max", "30");
    _mqttEntities.tempConfortZ1.set("mode", "box");
    _mqttEntities.tempConfortZ1.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempConfortZ1, true);
    mqtt().onCommand(_mqttEntities.tempConfortZ1, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température confort Z1 à %0.2f.", temperature);
            getZone1().setTemperatureConfort(temperature);
            _envoiZ1 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température réduite
    _mqttEntities.tempReduitZ1.id = "temperatureReduitZ1";
    _mqttEntities.tempReduitZ1.name = "Température Réduit Z1";
    _mqttEntities.tempReduitZ1.component = "number";
    _mqttEntities.tempReduitZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureReduit"}), 0, true);
    _mqttEntities.tempReduitZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureReduit", "set"}), 0, true);
    _mqttEntities.tempReduitZ1.set("device_class", "temperature");
    _mqttEntities.tempReduitZ1.set("state_class", "measurement");
    _mqttEntities.tempReduitZ1.set("unit_of_measurement", "°C");
    _mqttEntities.tempReduitZ1.set("min", "5");
    _mqttEntities.tempReduitZ1.set("max", "30");
    _mqttEntities.tempReduitZ1.set("mode", "box");
    _mqttEntities.tempReduitZ1.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempReduitZ1, true);
    mqtt().onCommand(_mqttEntities.tempReduitZ1, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température réduite Z1 à %0.2f.", temperature);
            getZone1().setTemperatureReduit(temperature);
            _envoiZ1 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température hors-gel
    _mqttEntities.tempHorsGelZ1.id = "temperatureHorsGelZ1";
    _mqttEntities.tempHorsGelZ1.name = "Température Hors-Gel Z1";
    _mqttEntities.tempHorsGelZ1.component = "number";
    _mqttEntities.tempHorsGelZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureHorsGel"}), 0, true);
    _mqttEntities.tempHorsGelZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureHorsGel", "set"}), 0, true);
    _mqttEntities.tempHorsGelZ1.set("device_class", "temperature");
    _mqttEntities.tempHorsGelZ1.set("state_class", "measurement");
    _mqttEntities.tempHorsGelZ1.set("unit_of_measurement", "°C");
    _mqttEntities.tempHorsGelZ1.set("min", "5");
    _mqttEntities.tempHorsGelZ1.set("max", "30");
    _mqttEntities.tempHorsGelZ1.set("mode", "box");
    _mqttEntities.tempHorsGelZ1.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempHorsGelZ1, true);
    mqtt().onCommand(_mqttEntities.tempHorsGelZ1, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température hors-gel Z1 à %0.2f.", temperature);
            getZone1().setTemperatureHorsGel(temperature);
            _envoiZ1 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température départ
    _mqttEntities.tempDepartZ1.id = "temperatureDepartZ1";
    _mqttEntities.tempDepartZ1.name = "Température Départ Z1";
    _mqttEntities.tempDepartZ1.component = "sensor";
    _mqttEntities.tempDepartZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureDepart"}), 0, true);
    _mqttEntities.tempDepartZ1.set("device_class", "temperature");
    _mqttEntities.tempDepartZ1.set("state_class", "measurement");
    _mqttEntities.tempDepartZ1.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempDepartZ1, true);

    // SENSOR: Température boost
    _mqttEntities.tempBoostZ1.id = "temperatureBoostZ1";
    _mqttEntities.tempBoostZ1.name = "Température Boost Z1";
    _mqttEntities.tempBoostZ1.component = "number";
    _mqttEntities.tempBoostZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureBoost"}), 0, true);
    _mqttEntities.tempBoostZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","temperatureBoost", "set"}), 0, true);
    _mqttEntities.tempBoostZ1.set("device_class", "temperature");
    _mqttEntities.tempBoostZ1.set("state_class", "measurement");
    _mqttEntities.tempBoostZ1.set("unit_of_measurement", "°C");
    _mqttEntities.tempBoostZ1.set("min", "0");
    _mqttEntities.tempBoostZ1.set("max", "30");
    _mqttEntities.tempBoostZ1.set("mode", "box");
    _mqttEntities.tempBoostZ1.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempBoostZ1, true);
    mqtt().onCommand(_mqttEntities.tempBoostZ1, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température de boost Z1 à %0.2f.", temperature);
            getZone1().setTemperatureBoost(temperature);
            if(getZone1().boostActif()) {
                _envoiZ1 = true;
                _lastEnvoiZone = 0;
            }
        }
    });

    // SWITCH: Activation Boost
    _mqttEntities.boostZ1.id = "boostZ1";
    _mqttEntities.boostZ1.name = "Boost Z1";
    _mqttEntities.boostZ1.component = "switch";
    _mqttEntities.boostZ1.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","boost"}), 0, true);
    _mqttEntities.boostZ1.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z1","boost", "set"}), 0, true);
    _mqttEntities.boostZ1.set("icon", "mdi:tune-variant");
    _mqttEntities.boostZ1.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.boostZ1, true);
    mqtt().onCommand(_mqttEntities.boostZ1, [&](const String& payload){
        payload.equalsIgnoreCase("ON") ? getZone1().activerBoost() : getZone1().desactiverBoost();
        _envoiZ1 = true;
        _lastEnvoiZone = 0;
    });


    // Zone 2

    _mqttEntities.modeZ2.id = "modeChauffageZ2";
    _mqttEntities.modeZ2.name = "Mode Chauffage Z2";
    _mqttEntities.modeZ2.component = "select";
    _mqttEntities.modeZ2.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"z2","mode"}), 0, true);
    _mqttEntities.modeZ2.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","mode","set"}), 0, true);
    _mqttEntities.modeZ2.set("icon", "mdi:tune-variant");
    _mqttEntities.modeZ2.set("entity_category", "config");
    _mqttEntities.modeZ2.setRaw("options", R"(["Hors Gel","Réduit","Confort","Auto"])");
    mqtt().registerEntity(*device, _mqttEntities.modeZ2, true);
    mqtt().onCommand(_mqttEntities.modeZ2, [&](const String& payload){
        getZone2().setMode(payload);
        _envoiZ2 = true;
        _lastEnvoiZone = 0;
    });

     // SENSOR: Température ambiante
    _mqttEntities.tempAmbianteZ2.id = "temperatureAmbianteZ2";
    _mqttEntities.tempAmbianteZ2.name = "Température ambiante Z2";
    _mqttEntities.tempAmbianteZ2.component = "sensor";
    _mqttEntities.tempAmbianteZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureAmbiante"}), 0, true);
    _mqttEntities.tempAmbianteZ2.set("device_class", "temperature");
    _mqttEntities.tempAmbianteZ2.set("state_class", "measurement");
    _mqttEntities.tempAmbianteZ2.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempAmbianteZ2, true);

    // SENSOR: Température consigne
    _mqttEntities.tempConsigneZ2.id = "temperatureConsigneZ2";
    _mqttEntities.tempConsigneZ2.name = "Consigne Z2";
    _mqttEntities.tempConsigneZ2.component = "sensor";
    _mqttEntities.tempConsigneZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureConsigne"}), 0, true);
    _mqttEntities.tempConsigneZ2.set("device_class", "temperature");
    _mqttEntities.tempConsigneZ2.set("state_class", "measurement");
    _mqttEntities.tempConsigneZ2.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempConsigneZ2, true);

    // SENSOR: Température confort
    _mqttEntities.tempConfortZ2.id = "temperatureConfortZ2";
    _mqttEntities.tempConfortZ2.name = "Température Confort Z2";
    _mqttEntities.tempConfortZ2.component = "number";
    _mqttEntities.tempConfortZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureConfort"}), 0, true);
    _mqttEntities.tempConfortZ2.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureConfort", "set"}), 0, true);
    _mqttEntities.tempConfortZ2.set("device_class", "temperature");
    _mqttEntities.tempConfortZ2.set("state_class", "measurement");
    _mqttEntities.tempConfortZ2.set("unit_of_measurement", "°C");
    _mqttEntities.tempConfortZ2.set("min", "5");
    _mqttEntities.tempConfortZ2.set("max", "30");
    _mqttEntities.tempConfortZ2.set("mode", "box");
    _mqttEntities.tempConfortZ2.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempConfortZ2, true);
    mqtt().onCommand(_mqttEntities.tempConfortZ2, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température confort Z2 à %0.2f.", temperature);
            getZone2().setTemperatureConfort(temperature);
            _envoiZ2 = true;
            _lastEnvoiZone = 0;
        }
    });
    

    // SENSOR: Température réduite
    _mqttEntities.tempReduitZ2.id = "temperatureReduitZ2";
    _mqttEntities.tempReduitZ2.name = "Température Réduit Z2";
    _mqttEntities.tempReduitZ2.component = "number";
    _mqttEntities.tempReduitZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureReduit"}), 0, true);
    _mqttEntities.tempReduitZ2.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureReduit", "set"}), 0, true);
    _mqttEntities.tempReduitZ2.set("device_class", "temperature");
    _mqttEntities.tempReduitZ2.set("state_class", "measurement");
    _mqttEntities.tempReduitZ2.set("unit_of_measurement", "°C");
    _mqttEntities.tempReduitZ2.set("min", "5");
    _mqttEntities.tempReduitZ2.set("max", "30");
    _mqttEntities.tempReduitZ2.set("mode", "box");
    _mqttEntities.tempReduitZ2.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempReduitZ2, true);
    mqtt().onCommand(_mqttEntities.tempReduitZ2, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température réduite Z2 à %0.2f.", temperature);
            getZone2().setTemperatureReduit(temperature);
            _envoiZ2 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température hors-gel
    _mqttEntities.tempHorsGelZ2.id = "temperatureHorsGelZ2";
    _mqttEntities.tempHorsGelZ2.name = "Température Hors-Gel Z2";
    _mqttEntities.tempHorsGelZ2.component = "number";
    _mqttEntities.tempHorsGelZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureHorsGel"}), 0, true);
    _mqttEntities.tempHorsGelZ2.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureHorsGel", "set"}), 0, true);
    _mqttEntities.tempHorsGelZ2.set("device_class", "temperature");
    _mqttEntities.tempHorsGelZ2.set("state_class", "measurement");
    _mqttEntities.tempHorsGelZ2.set("unit_of_measurement", "°C");
    _mqttEntities.tempHorsGelZ2.set("min", "5");
    _mqttEntities.tempHorsGelZ2.set("max", "30");
    _mqttEntities.tempHorsGelZ2.set("mode", "box");
    _mqttEntities.tempHorsGelZ2.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempHorsGelZ2, true);
    mqtt().onCommand(_mqttEntities.tempHorsGelZ2, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température hors-gel Z2 à %0.2f.", temperature);
            getZone2().setTemperatureHorsGel(temperature);
            _envoiZ2 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température départ
    _mqttEntities.tempDepartZ2.id = "temperatureDepartZ2";
    _mqttEntities.tempDepartZ2.name = "Température Départ Z2";
    _mqttEntities.tempDepartZ2.component = "sensor";
    _mqttEntities.tempDepartZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureDepart"}), 0, true);
    _mqttEntities.tempDepartZ2.set("device_class", "temperature");
    _mqttEntities.tempDepartZ2.set("state_class", "measurement");
    _mqttEntities.tempDepartZ2.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempDepartZ2, true);

    // SENSOR: Température boost
    _mqttEntities.tempBoostZ2.id = "temperatureBoostZ2";
    _mqttEntities.tempBoostZ2.name = "Température Boost Z2";
    _mqttEntities.tempBoostZ2.component = "number";
    _mqttEntities.tempBoostZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureBoost"}), 0, true);
    _mqttEntities.tempBoostZ2.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","temperatureBoost", "set"}), 0, true);
    _mqttEntities.tempBoostZ2.set("device_class", "temperature");
    _mqttEntities.tempBoostZ2.set("state_class", "measurement");
    _mqttEntities.tempBoostZ2.set("unit_of_measurement", "°C");
    _mqttEntities.tempBoostZ2.set("min", "0");
    _mqttEntities.tempBoostZ2.set("max", "30");
    _mqttEntities.tempBoostZ2.set("mode", "box");
    _mqttEntities.tempBoostZ2.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempBoostZ2, true);
    mqtt().onCommand(_mqttEntities.tempBoostZ2, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température de boost Z2 à %0.2f.", temperature);
            getZone2().setTemperatureBoost(temperature);
            if(getZone2().boostActif()) {
                _envoiZ2 = true;
                _lastEnvoiZone = 0;
            }
        }
    });

    // SWITCH: Activation Boost
    _mqttEntities.boostZ2.id = "boostZ2";
    _mqttEntities.boostZ2.name = "Boost Z2";
    _mqttEntities.boostZ2.component = "switch";
    _mqttEntities.boostZ2.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","boost"}), 0, true);
    _mqttEntities.boostZ2.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z2","boost", "set"}), 0, true);
    _mqttEntities.boostZ2.set("icon", "mdi:tune-variant");
    _mqttEntities.boostZ2.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.boostZ2, true);
    mqtt().onCommand(_mqttEntities.boostZ2, [&](const String& payload){
        payload.equalsIgnoreCase("ON") ? getZone2().activerBoost() : getZone2().desactiverBoost();
        _envoiZ2 = true;
        _lastEnvoiZone = 0;
    });

    // Zone 3

    _mqttEntities.modeZ3.id = "modeChauffageZ3";
    _mqttEntities.modeZ3.name = "Mode Chauffage Z3";
    _mqttEntities.modeZ3.component = "select";
    _mqttEntities.modeZ3.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"z3","mode"}), 0, true);
    _mqttEntities.modeZ3.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","mode","set"}), 0, true);
    _mqttEntities.modeZ3.set("icon", "mdi:tune-variant");
    _mqttEntities.modeZ3.set("entity_category", "config");
    _mqttEntities.modeZ3.setRaw("options", R"(["Hors Gel","Réduit","Confort","Auto"])");
    mqtt().registerEntity(*device, _mqttEntities.modeZ3, true);
    mqtt().onCommand(_mqttEntities.modeZ3, [&](const String& payload){
        getZone3().setMode(payload);
        _envoiZ3 = true;
        _lastEnvoiZone = 0;
    });

     // SENSOR: Température ambiante
    _mqttEntities.tempAmbianteZ3.id = "temperatureAmbianteZ3";
    _mqttEntities.tempAmbianteZ3.name = "Température ambiante Z3";
    _mqttEntities.tempAmbianteZ3.component = "sensor";
    _mqttEntities.tempAmbianteZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureAmbiante"}), 0, true);
    _mqttEntities.tempAmbianteZ3.set("device_class", "temperature");
    _mqttEntities.tempAmbianteZ3.set("state_class", "measurement");
    _mqttEntities.tempAmbianteZ3.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempAmbianteZ3, true);

    // SENSOR: Température consigne
    _mqttEntities.tempConsigneZ3.id = "temperatureConsigneZ3";
    _mqttEntities.tempConsigneZ3.name = "Consigne Z3";
    _mqttEntities.tempConsigneZ3.component = "sensor";
    _mqttEntities.tempConsigneZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureConsigne"}), 0, true);
    _mqttEntities.tempConsigneZ3.set("device_class", "temperature");
    _mqttEntities.tempConsigneZ3.set("state_class", "measurement");
    _mqttEntities.tempConsigneZ3.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempConsigneZ3, true);

    // SENSOR: Température confort
    _mqttEntities.tempConfortZ3.id = "temperatureConfortZ3";
    _mqttEntities.tempConfortZ3.name = "Température Confort Z3";
    _mqttEntities.tempConfortZ3.component = "number";
    _mqttEntities.tempConfortZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureConfort"}), 0, true);
    _mqttEntities.tempConfortZ3.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureConfort", "set"}), 0, true);
    _mqttEntities.tempConfortZ3.set("device_class", "temperature");
    _mqttEntities.tempConfortZ3.set("state_class", "measurement");
    _mqttEntities.tempConfortZ3.set("unit_of_measurement", "°C");
    _mqttEntities.tempConfortZ3.set("min", "5");
    _mqttEntities.tempConfortZ3.set("max", "30");
    _mqttEntities.tempConfortZ3.set("mode", "box");
    _mqttEntities.tempConfortZ3.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempConfortZ3, true);
    mqtt().onCommand(_mqttEntities.tempConfortZ3, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température confort Z3 à %0.2f.", temperature);
            getZone3().setTemperatureConfort(temperature);
            _envoiZ3 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température réduite
    _mqttEntities.tempReduitZ3.id = "temperatureReduitZ3";
    _mqttEntities.tempReduitZ3.name = "Température Réduit Z3";
    _mqttEntities.tempReduitZ3.component = "number";
    _mqttEntities.tempReduitZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureReduit"}), 0, true);
    _mqttEntities.tempReduitZ3.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureReduit", "set"}), 0, true);
    _mqttEntities.tempReduitZ3.set("device_class", "temperature");
    _mqttEntities.tempReduitZ3.set("state_class", "measurement");
    _mqttEntities.tempReduitZ3.set("unit_of_measurement", "°C");
    _mqttEntities.tempReduitZ3.set("min", "5");
    _mqttEntities.tempReduitZ3.set("max", "30");
    _mqttEntities.tempReduitZ3.set("mode", "box");
    _mqttEntities.tempReduitZ3.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempReduitZ3, true);
    mqtt().onCommand(_mqttEntities.tempReduitZ3, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température réduite Z3 à %0.2f.", temperature);
            getZone3().setTemperatureReduit(temperature);
            _envoiZ3 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température hors-gel
    _mqttEntities.tempHorsGelZ3.id = "temperatureHorsGelZ3";
    _mqttEntities.tempHorsGelZ3.name = "Température Hors-Gel Z3";
    _mqttEntities.tempHorsGelZ3.component = "number";
    _mqttEntities.tempHorsGelZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureHorsGel"}), 0, true);
    _mqttEntities.tempHorsGelZ3.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureHorsGel", "set"}), 0, true);
    _mqttEntities.tempHorsGelZ3.set("device_class", "temperature");
    _mqttEntities.tempHorsGelZ3.set("state_class", "measurement");
    _mqttEntities.tempHorsGelZ3.set("unit_of_measurement", "°C");
    _mqttEntities.tempHorsGelZ3.set("min", "5");
    _mqttEntities.tempHorsGelZ3.set("max", "30");
    _mqttEntities.tempHorsGelZ3.set("mode", "box");
    _mqttEntities.tempHorsGelZ3.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempHorsGelZ3, true);
    mqtt().onCommand(_mqttEntities.tempHorsGelZ3, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température hors-gel Z3 à %0.2f.", temperature);
            getZone3().setTemperatureHorsGel(temperature);
            _envoiZ3 = true;
            _lastEnvoiZone = 0;
        }
    });

    // SENSOR: Température départ
    _mqttEntities.tempDepartZ3.id = "temperatureDepartZ3";
    _mqttEntities.tempDepartZ3.name = "Température Départ Z3";
    _mqttEntities.tempDepartZ3.component = "sensor";
    _mqttEntities.tempDepartZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureDepart"}), 0, true);
    _mqttEntities.tempDepartZ3.set("device_class", "temperature");
    _mqttEntities.tempDepartZ3.set("state_class", "measurement");
    _mqttEntities.tempDepartZ3.set("unit_of_measurement", "°C");
    mqtt().registerEntity(*device, _mqttEntities.tempDepartZ3, true);

    // SENSOR: Température boost
    _mqttEntities.tempBoostZ3.id = "temperatureBoostZ3";
    _mqttEntities.tempBoostZ3.name = "Température Boost Z3";
    _mqttEntities.tempBoostZ3.component = "number";
    _mqttEntities.tempBoostZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureBoost"}), 0, true);
    _mqttEntities.tempBoostZ3.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","temperatureBoost", "set"}), 0, true);
    _mqttEntities.tempBoostZ3.set("device_class", "temperature");
    _mqttEntities.tempBoostZ3.set("state_class", "measurement");
    _mqttEntities.tempBoostZ3.set("unit_of_measurement", "°C");
    _mqttEntities.tempBoostZ3.set("min", "0");
    _mqttEntities.tempBoostZ3.set("max", "30");
    _mqttEntities.tempBoostZ3.set("mode", "box");
    _mqttEntities.tempBoostZ3.set("step", "0.5");
    mqtt().registerEntity(*device, _mqttEntities.tempBoostZ3, true);
    mqtt().onCommand(_mqttEntities.tempBoostZ3, [&](const String& payload) {
        float temperature = payload.toFloat();
        if(!isnan(temperature)) {
            info("[CONNECT] Modification de la température de boost Z3 à %0.2f.", temperature);
            getZone3().setTemperatureBoost(temperature);
            if(getZone3().boostActif()) {
                _envoiZ3 = true;
                _lastEnvoiZone = 0;
            }
        }
    });

    // SWITCH: Activation Boost
    _mqttEntities.boostZ3.id = "boostZ3";
    _mqttEntities.boostZ3.name = "Boost Z3";
    _mqttEntities.boostZ3.component = "switch";
    _mqttEntities.boostZ3.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","boost"}), 0, true);
    _mqttEntities.boostZ3.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"z3","boost", "set"}), 0, true);
    _mqttEntities.boostZ3.set("icon", "mdi:tune-variant");
    _mqttEntities.boostZ3.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.boostZ3, true);
    mqtt().onCommand(_mqttEntities.boostZ3, [&](const String& payload){
        payload.equalsIgnoreCase("ON") ? getZone3().activerBoost() : getZone3().desactiverBoost();
        _envoiZ3 = true;
        _lastEnvoiZone = 0;
    });

  // SENSOR: Température ECS
  _mqttEntities.tempECS.id = "temperatureECS";
  _mqttEntities.tempECS.name = "Température ECS";
  _mqttEntities.tempECS.component = "sensor";
  _mqttEntities.tempECS.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "connect", "temperatureECS"}), 0, true);
  _mqttEntities.tempECS.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic, "connect", "temperatureECS", "set"}), 0, true);
  _mqttEntities.tempECS.set("device_class", "temperature");
  _mqttEntities.tempECS.set("state_class", "measurement");
  _mqttEntities.tempECS.set("unit_of_measurement", "°C");
  mqtt().registerEntity(*device, _mqttEntities.tempECS, true);

  // SENSOR: Température CDC
  _mqttEntities.tempCDC.id = "temperatureCDC";
  _mqttEntities.tempCDC.name = "Température CDC";
  _mqttEntities.tempCDC.component = "sensor";
  _mqttEntities.tempCDC.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "connect", "temperatureCDC"}), 0, true);
  _mqttEntities.tempCDC.set("device_class", "temperature");
  _mqttEntities.tempCDC.set("state_class", "measurement");
  _mqttEntities.tempCDC.set("unit_of_measurement", "°C");
  mqtt().registerEntity(*device, _mqttEntities.tempCDC, true);

  // SENSOR: Température extérieure
  _mqttEntities.tempExterieure.id = "temperatureExterieureConnect";
  _mqttEntities.tempExterieure.name = "Température extérieure";
  _mqttEntities.tempExterieure.component = "sensor";
  _mqttEntities.tempExterieure.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "connect", "temperatureExterieure"}), 0, true);
  _mqttEntities.tempExterieure.set("device_class", "temperature");
  _mqttEntities.tempExterieure.set("state_class", "measurement");
  _mqttEntities.tempExterieure.set("unit_of_measurement", "°C");
  mqtt().registerEntity(*device, _mqttEntities.tempExterieure, true);

  // SENSOR: Consommation chauffage
  _mqttEntities.consommationChauffage.id = "consommationChauffage";
  _mqttEntities.consommationChauffage.name = "Consommation chauffage";
  _mqttEntities.consommationChauffage.component = "sensor";
  _mqttEntities.consommationChauffage.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "connect", "consommationChauffage"}), 0, true);
  _mqttEntities.consommationChauffage.set("device_class", "energy");
  _mqttEntities.consommationChauffage.set("state_class", "total_increasing");
  _mqttEntities.consommationChauffage.set("unit_of_measurement", "kWh");
  mqtt().registerEntity(*device, _mqttEntities.consommationChauffage, true);

  // SENSOR: Consommation ECS
  _mqttEntities.consommationECS.id = "consommationECS";
  _mqttEntities.consommationECS.name = "Consommation ECS";
  _mqttEntities.consommationECS.component = "sensor";
  _mqttEntities.consommationECS.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "connect", "consommationECS"}), 0, true);
  _mqttEntities.consommationECS.set("device_class", "energy");
  _mqttEntities.consommationECS.set("state_class", "total_increasing");
  _mqttEntities.consommationECS.set("unit_of_measurement", "kWh");
  mqtt().registerEntity(*device, _mqttEntities.consommationECS, true);
}

void Connect::loop() {
    uint32_t now = millis();

    if(estAssocie()) {
        if (now - _lastRecuperationTemperatures >= 300000 || _lastRecuperationTemperatures == 0) { // 5 minutes
            info("[CONNECT] Récupération des températures...");
            if(recupererTemperatures()) {
                _lastRecuperationTemperatures = now;
                publishMqtt();
            } else {
                _lastRecuperationTemperatures = now <= 60000 ? 1 : now - 60000;
                error("[CONNECT] Échec de la récupération des températures.");
            }
        }

        if (now - _lastRecuperationConsommation >= 3600000 || _lastRecuperationConsommation == 0) { // 1 heure
            info("[CONNECT] Récupération des consommations...");
            if(recupererConsommation()) {
                _lastRecuperationConsommation = now;
                publishMqtt();
            } else {
                _lastRecuperationConsommation = now <= 60000 ? 1 : now - 60000;
            }
        }

        if (now - _lastEnvoiZone >= 60000 || _lastEnvoiZone == 0) { // 1 minute
            envoiZone();
        }
    }
}

void Connect::envoiZone() {
    if(estAssocie()) {
        if(_envoiZ1) {
            info("[CONNECT] Envoi de la zone 1.");
            if(envoyerZone(_zone1)) {
                info("[CONNECT] Envoi réussi !");
                publishMqtt();
                _envoiZ1 = false;
            }
        }
        if(_envoiZ2) {
            info("[CONNECT] Envoi de la zone 2.");
            if(envoyerZone(_zone2)) {
                info("[CONNECT] Envoi réussi !");
                publishMqtt();
                _envoiZ2 = false;
            }
        }
        if(_envoiZ3) {
            info("[CONNECT] Envoi de la zone 3.");
            if(envoyerZone(_zone3)) {
                info("[CONNECT] Envoi réussi !");
                publishMqtt();
                _envoiZ3 = false;
            }
        }
        _lastEnvoiZone = millis();
    }
}

void Connect::publishMqtt() {
    if( !isnan(getTemperatureECS())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureECS"), getTemperatureECS());
    }
    if( !isnan(getTemperatureCDC())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureCDC"), getTemperatureCDC());
    }
    if( !isnan(getTemperatureExterieure())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureExterieureConnect"), getTemperatureExterieure());
    }

    if( getConsommationChauffage() >= 0) {
        static float lastConsommationChauffage = -1;
        if(lastConsommationChauffage != getConsommationChauffage()) {
            lastConsommationChauffage = getConsommationChauffage();
            mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("consommationChauffage"), getConsommationChauffage());
        }
    }
    if( getConsommationECS() >= 0 && getConsommationECS()) {
        static float lastConsommationECS = -1;
        if(lastConsommationECS != getConsommationECS()) {
            lastConsommationECS = getConsommationECS();
            mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("consommationECS"), getConsommationECS());
        }
    }

    // Zone 1
    if(!isnan(getZone1().getTemperatureAmbiante())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureAmbianteZ1"), getZone1().getTemperatureAmbiante());
    }
    if(!isnan(getZone1().getTemperatureConsigne())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConsigneZ1"), getZone1().getTemperatureConsigne());
    }
    if(!isnan(getZone1().getTemperatureDepart())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureDepartZ1"), getZone1().getTemperatureDepart());
    }
    if(!isnan(getZone1().getTemperatureConfort())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConfortZ1"), getZone1().getTemperatureConfort());
    }
    if(!isnan(getZone1().getTemperatureReduit())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureReduitZ1"), getZone1().getTemperatureReduit());
    }
    if(!isnan(getZone1().getTemperatureHorsGel())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureHorsGelZ1"), getZone1().getTemperatureHorsGel());
    }
    if(!isnan(getZone1().getTemperatureBoost())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureBoostZ1"), getZone1().getTemperatureBoost());
    }
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("boostZ1"), getZone1().boostActif() ? "ON" : "OFF");
    if(getZone1().getMode() != Zone::MODE_ZONE::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeChauffageZ1"), getZone1().getNomMode().c_str());
    }


    // Zone 2
    if(!isnan(getZone2().getTemperatureAmbiante())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureAmbianteZ2"), getZone2().getTemperatureAmbiante());
    }
    if(!isnan(getZone2().getTemperatureConsigne())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConsigneZ2"), getZone2().getTemperatureConsigne());
    }
    if(!isnan(getZone2().getTemperatureDepart() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureDepartZ2"), getZone2().getTemperatureDepart());
    }
    if(!isnan(getZone2().getTemperatureConfort() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConfortZ2"), getZone2().getTemperatureConfort());
    }
    if(!isnan(getZone2().getTemperatureReduit() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureReduitZ2"), getZone2().getTemperatureReduit());
    }
    if(!isnan(getZone2().getTemperatureHorsGel() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureHorsGelZ2"), getZone2().getTemperatureHorsGel());
    }
    if(!isnan(getZone2().getTemperatureBoost())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureBoostZ2"), getZone2().getTemperatureBoost());
    }
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("boostZ2"), getZone2().boostActif() ? "ON" : "OFF");
    if(getZone2().getMode() != Zone::MODE_ZONE::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeChauffageZ2"), getZone2().getNomMode().c_str());
    }

    // Zone 3
    if(!isnan(getZone3().getTemperatureAmbiante() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureAmbianteZ3"), getZone3().getTemperatureAmbiante());
    }
    if(!isnan(getZone3().getTemperatureConsigne() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConsigneZ3"), getZone3().getTemperatureConsigne());
    }
    if(!isnan(getZone3().getTemperatureDepart() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureDepartZ3"), getZone3().getTemperatureDepart());
    }
    if(!isnan(getZone3().getTemperatureConfort() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureConfortZ3"), getZone3().getTemperatureConfort());
    }
    if(!isnan(getZone3().getTemperatureReduit() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureReduitZ3"), getZone3().getTemperatureReduit());
    }
    if(!isnan(getZone3().getTemperatureHorsGel() )) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureHorsGelZ3"), getZone3().getTemperatureHorsGel());
    }
    if(!isnan(getZone3().getTemperatureBoost())) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureBoostZ3"), getZone3().getTemperatureBoost());
    }
    mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("boostZ3"), getZone3().boostActif() ? "ON" : "OFF");
    if(getZone3().getMode() != Zone::MODE_ZONE::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeChauffageZ3"), getZone3().getNomMode().c_str());
    }
}

// ====== Zone ========
void Connect::Zone::setTemperatureConfort(float temperature) {
    this->_temperatureConfort = std::min(30.0f, std::max(5.0f, temperature));
}
void Connect::Zone::setTemperatureReduit(float temperature) {
    this->_temperatureReduit = std::min(30.0f, std::max(5.0f, temperature));
}
void Connect::Zone::setTemperatureHorsGel(float temperature) {
    this->_temperatureHorsGel = std::min(30.0f, std::max(5.0f, temperature));
}
void Connect::Zone::setTemperatureAmbiante(float temperature) {
    this->_temperatureAmbiante = temperature;
}
void Connect::Zone::setTemperatureConsigne(float temperature) {
    this->_temperatureConsigne = std::min(30.0f, std::max(5.0f, temperature));
}
void Connect::Zone::setTemperatureBoost(float temperature) {
    //this->_temperatureBoost = std::min(30.0f, std::max(5.0f, temperature));
    this->_temperatureBoost = std::min(5.0f, std::max(0.5f, temperature));
}
void Connect::Zone::setTemperatureDepart(float temperature) {
    this->_temperatureDepart = temperature;
}

float Connect::Zone::getTemperatureConfort() {
    return this->_temperatureConfort;
}
float Connect::Zone::getTemperatureReduit() {
    return this->_temperatureReduit;
}
float Connect::Zone::getTemperatureHorsGel() {
    return this->_temperatureHorsGel;
}
float Connect::Zone::getTemperatureConsigne() {
    return this->_temperatureConsigne;
}
float Connect::Zone::getTemperatureAmbiante() {
    return this->_temperatureAmbiante;
}
float Connect::Zone::getTemperatureDepart() {
    return this->_temperatureDepart;
}
float Connect::Zone::getTemperatureBoost() {
    return this->_temperatureBoost;
}

Connect::Zone::MODE_ZONE Connect::Zone::getMode() {
    return (MODE_ZONE)this->_mode;
}

void Connect::Zone::setMode(MODE_ZONE mode) {
    this->_mode = mode;
}
void Connect::Zone::setMode(const String& mode) {
    if (mode.equalsIgnoreCase("Auto")) {
    this->setMode(MODE_ZONE::AUTO);
  } else if (mode.equalsIgnoreCase("Réduit")) {
    this->setMode(MODE_ZONE::REDUIT);
  } else if (mode.equalsIgnoreCase("Hors gel")) {
    this->setMode(MODE_ZONE::HORS_GEL);
  } else if (mode.equalsIgnoreCase("Confort")) {
    this->setMode(MODE_ZONE::CONFORT);
  }
}

String Connect::Zone::getNomMode() {
    switch(this->getMode()) {
        case MODE_ZONE::AUTO:
            return "Auto";
            break;
        case MODE_ZONE::CONFORT:
            return "Confort";
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

byte Connect::Zone::getModeOptions() {
    return this->_modeOptions;
}
void Connect::Zone::setModeOptions(byte modeOptions) {
    this->_modeOptions = modeOptions;
}

uint8_t Connect::Zone::getIdZone() {
    return this->_idZone;
}

void Connect::Zone::activerBoost() {
    //TODO Test si confort ?
    _modeOptions |= 0b01000000;
}
void Connect::Zone::desactiverBoost() {
    _modeOptions &= ~0b01000000;
}
bool Connect::Zone::boostActif() {
    return (_modeOptions & 0b01000000) != 0;
}