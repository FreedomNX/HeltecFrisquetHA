#include "Connect.h"
#include "../Buffer.h"
#include <math.h>

void Connect::loadConfig() {
    bool checkMigration = false;

    getPreferences().begin("connectCfg", false);

    if(! getPreferences().isKey("idAssociation")) {
        checkMigration = true;
    }

    setIdAssociation(getPreferences().getUChar("idAssociation", 0xFF));

    getPreferences().end();
    delay(100);
    
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
    getPreferences().end();
}

bool Connect::envoyerZone(Zone& zone) {
    if(! estAssocie()) {
        return false;
    }

    if(isnan(zone.getTemperatureConfort()) || isnan(zone.getTemperatureReduit()) || isnan(zone.getTemperatureHorsGel()) || zone.getMode() == Zone::MODE_ZONE::INCONNU) {
        error("[CONNECT] Impossible d'envoyer la zone %d, configuration incomplète.", zone.getNumeroZone());
        return false;
    }

    info("[CONNECT] Envoi de la zone %d.", zone.getNumeroZone());
    return true;//TODO A SUPPRIMER

    if(zone.getIdZone() == ID_ZONE_1 && getConfig().useSatelliteVirtualZ1()) {
        return true;
    } else if(zone.getIdZone() == ID_ZONE_2 && getConfig().useSatelliteVirtualZ2()) {
        return true;
    } else if(zone.getIdZone() == ID_ZONE_3 && getConfig().useSatelliteVirtualZ3()) {
        return true;
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

        if(getZone1().getSource() == Zone::SOURCE::CONNECT) {
            getZone1().setTemperatureAmbiante(buff.temperatureAmbianteZ1.toFloat());
            getZone1().setTemperatureConsigne(buff.temperatureConsigneZ1.toFloat());
        }
        getZone1().setTemperatureDepart(buff.temperatureDepartZ1.toFloat());

        if(getZone2().getSource() == Zone::SOURCE::CONNECT) {
            getZone2().setTemperatureAmbiante(buff.temperatureAmbianteZ2.toFloat());
            getZone2().setTemperatureConsigne(buff.temperatureConsigneZ2.toFloat());
        }
        getZone2().setTemperatureDepart(buff.temperatureDepartZ2.toFloat());

        if(getZone3().getSource() == Zone::SOURCE::CONNECT) {
            getZone3().setTemperatureAmbiante(buff.temperatureAmbianteZ3.toFloat());
            getZone3().setTemperatureConsigne(buff.temperatureConsigneZ3.toFloat());
        }
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
    

    size_t length = 0;
    uint16_t err;

    uint8_t retry = 0;
    do {
        //length = sizeof(buff);
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
            delay(10);
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
String Connect::getNomModeECS() {
    switch(getModeECS()) {
        case MODE_ECS::STOP:
            return "Stop";
            break;
        case MODE_ECS::MAX:
            return "Max";
            break;
        case MODE_ECS::ECO:
            return "Eco";
            break;
        case MODE_ECS::ECO_HORAIRES:
            return "Eco Horaires";
            break;
        case MODE_ECS::ECOPLUS:
            return "Eco+";
            break;
        case MODE_ECS::ECOPLUS_HORAIRES:
            return "Eco+ Horaires";
            break;
    }

    return "Inconnu";
}
bool Connect::setModeECS(MODE_ECS modeECS) {
    _modeECS = modeECS;
    return true;
}
bool Connect::setModeECS(const String& modeECS) {
    if (modeECS.equalsIgnoreCase("Max")) {
        this->setModeECS(MODE_ECS::MAX);
    } else if (modeECS.equalsIgnoreCase("Eco")) {
        this->setModeECS(MODE_ECS::ECO);
    } else if (modeECS.equalsIgnoreCase("Eco+")) {
        this->setModeECS(MODE_ECS::ECOPLUS);
    } else if (modeECS.equalsIgnoreCase("Eco Horaires")) {
        this->setModeECS(MODE_ECS::ECO_HORAIRES);
    } else if (modeECS.equalsIgnoreCase("Eco+ Horaires")) {
        this->setModeECS(MODE_ECS::ECOPLUS_HORAIRES);
    } else if (modeECS.equalsIgnoreCase("Stop")) {
        this->setModeECS(MODE_ECS::STOP);
    } else {
        return false;
    }

    return true;
}

bool Connect::envoyerModeECS() {
    if(! estAssocie()) {
        return false;
    }

    info("[CONNECT] Envoi du mode  ECS.");
    
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
        
        info("[CONNECT] Envoi réussie.");
        return true;
    } while(retry++ < 1);

    info("[CONNECT] Échec de l'envoi.");
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
  _mqttEntities.tempExterieure.id = "temperatureExterieure";
  _mqttEntities.tempExterieure.name = "Température extérieure";
  _mqttEntities.tempExterieure.component = "sensor";
  _mqttEntities.tempExterieure.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic, "sondeExterieure", "temperatureExterieure"}), 0, true);
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

   // SELECT: Mode ECS
    _mqttEntities.modeECS.id = "modeECS";
    _mqttEntities.modeECS.name = "Mode ECS";
    _mqttEntities.modeECS.component = "select";
    _mqttEntities.modeECS.stateTopic   = MqttTopic(MqttManager::compose({device->baseTopic,"connect", "modeECS"}), 0, true);
    _mqttEntities.modeECS.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"connect", "modeECS","set"}), 0, true);
    _mqttEntities.modeECS.set("icon", "mdi:tune-variant");
    _mqttEntities.modeECS.set("entity_category", "config");
    _mqttEntities.modeECS.setRaw("options", R"(["Max","Eco","Eco Horaires","Eco+", "Eco+ Horaires", "Stop"])");
    mqtt().registerEntity(*device, _mqttEntities.modeECS, true);
    mqtt().onCommand(_mqttEntities.modeECS, [&](const String& payload){
        info("[CONNECT] Changement du mode  ECS : %s.", payload.c_str());
        setModeECS(payload);
        if(envoyerModeECS()) {
            mqtt().publishState(_mqttEntities.modeECS, getNomModeECS());
        }
    });
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
            delay(100);
        }

        if (now - _lastRecuperationConsommation >= 3600000 || _lastRecuperationConsommation == 0) { // 1 heure
            info("[CONNECT] Récupération des consommations...");
            if(recupererConsommation()) {
                _lastRecuperationConsommation = now;
                publishMqtt();
            } else {
                _lastRecuperationConsommation = now <= 60000 ? 1 : now - 60000;
            }
            delay(100);
        }

        if (now - _lastRecuperationModeECS >= 3600000 || _lastRecuperationModeECS == 0) { // 1 heure
            info("[CONNECT] Récupération du mode ECS...");
            if(recupererModeECS()) {
                publishMqtt();
            } else {
                error("[CONNECT] Récupération impossible");
            }
            _lastRecuperationModeECS = now;
            delay(100);
        }

        if (now - _lastEnvoiZone >= 60000 || _lastEnvoiZone == 0) { // 1 minute
            envoiZone();
        }
    }
}

void Connect::envoiZone() {
    if(estAssocie()) {
        if(getZone1().getSource() == Zone::SOURCE::CONNECT && _zone1.getLastChange() > _zone1.getLastEnvoi()) {
            info("[CONNECT] Envoi de la zone 1.");
            if(envoyerZone(_zone1)) {
                info("[CONNECT] Envoi réussi !");
                _zone1.publishMqtt();
                _zone1.refreshLastEnvoi();
                _envoiZ1 = false;
            }
        }
        if(getZone1().getSource() == Zone::SOURCE::CONNECT && _zone2.getLastChange() > _zone3.getLastEnvoi()) {
            info("[CONNECT] Envoi de la zone 2.");
            if(envoyerZone(_zone2)) {
                info("[CONNECT] Envoi réussi !");
                _zone2.publishMqtt();
                _zone2.refreshLastEnvoi();
                _envoiZ2 = false;
            }
        }
        if(getZone1().getSource() == Zone::SOURCE::CONNECT && _zone3.getLastChange() > _zone3.getLastEnvoi()) {
            info("[CONNECT] Envoi de la zone 3.");
            if(envoyerZone(_zone3)) {
                info("[CONNECT] Envoi réussi !");
                _zone3.publishMqtt();
                _zone3.refreshLastEnvoi();
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
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("temperatureExterieure"), getTemperatureExterieure());
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

     if(getModeECS() != MODE_ECS::INCONNU) {
        mqtt().publishState(*mqtt().getDevice("heltecFrisquet")->getEntity("modeECS"), getNomModeECS().c_str());
    }
}
