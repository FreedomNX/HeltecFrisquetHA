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
    _mqttEntities.temperatureBoost.name = "Température Boost Z" + String(getNumeroZone());
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
        }
    });

    _mqttEntities.temperatureAmbiance.id = "tempAmbSatZ" + String(getNumeroZone());
    _mqttEntities.temperatureAmbiance.name = "Température Ambiance Z" + String(getNumeroZone());
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
    _mqttEntities.boost.name = "Boost Z" + String(getNumeroZone());
    _mqttEntities.boost.component = "switch";
    _mqttEntities.boost.stateTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"boost"}), 0, true);
    _mqttEntities.boost.commandTopic = MqttTopic(MqttManager::compose({device->baseTopic,"satellite", "z" + String(getNumeroZone()),"boost", "set"}), 0, true);
    _mqttEntities.boost.set("icon", "mdi:tune-variant");
    _mqttEntities.boost.set("entity_category", "config");
    mqtt().registerEntity(*device, _mqttEntities.boost, true);
    mqtt().onCommand(_mqttEntities.boost, [&](const String& payload){
        payload.equalsIgnoreCase("ON") ? activerBoost() : desactiverBoost();
        mqtt().publishState(_mqttEntities.boost, boostActif() ? "ON" : "OFF");
    });
}

void Satellite::loop() {
   uint32_t now = millis();

    if(estAssocie()) {
    }
}

void Satellite::publishMqtt() {
}

bool Satellite::onReceive(byte* donnees, size_t length) {
    if(! estAssocie()) {
        return false;
    }
    //Envoi 80 08 AB 0C 01 17 A0 29 00 15 A0 2F 00 04 08 00 CD 00 B9 00 05 00 00
    //Reponse  08 80 AB 10 81 17 2A 00 29 00 00 25 11 27 13 50 12 28 04 00 CD 00 B9 00 05 00 00 00 00 04 F6 00 00 00 00 00 00 00 00 04 F6 00 00 00 00 00 00 00 00
    
    ReadBuffer readBuffer(donnees, length);

    FrisquetRadio::RadioTrameHeader header;
    if(readBuffer.remainingLength() < sizeof(header)) { return false; }
    readBuffer.getBytes((byte*)&header, sizeof(header));

    if(_modeEcrasement) {
        if(header.type == FrisquetRadio::MessageType::INIT && header.idExpediteur == this->getId() && header.idMessage != getIdMessage()) { // Récéption en écoute
            FrisquetRadio::RadioTrameInit requete;
            if(readBuffer.remainingLength() < sizeof(requete)) { return false; }
            readBuffer.getBytes((byte*)&requete, sizeof(requete));

            if(requete.adresseMemoireEcriture.toUInt16() == 0xA02F && requete.tailleMemoireEcriture.toUInt16() == 0x0004) { // Envoi consigne
                info("[SATELLITE] Intercéption envoi consigne");

                struct {
                    temperature16 temperatureAmbiante; 
                    temperature16 temperatureConsigne;
                    uint8_t i1 = 0x00; 
                    uint8_t mode = 0x00; // 0x01 Confort, 0x02 Reduit, etc.
                    uint8_t i2[2] = {0};
                } donneesZone;
                if(readBuffer.remainingLength() < sizeof(donneesZone)) { return false; }
                readBuffer.getBytes((byte*)&donneesZone, sizeof(donneesZone));


                setIdAssociation(header.idAssociation);
                setIdMessage(header.idMessage);
                setMode((MODE)donneesZone.mode);
                setTemperatureAmbiante(donneesZone.temperatureAmbiante.toFloat());
                setTemperatureConsigne(donneesZone.temperatureConsigne.toFloat());

                saveConfig();

                uint8_t retry = 0;
                uint16_t err;
                
                do {
                    size_t lengthRx = 0;
                    byte donneesZones[RADIOLIB_SX126X_MAX_PACKET_LENGTH];

                    err = radio().receiveExpected(
                        header.idExpediteur, 
                        header.idDestinataire,
                        header.idAssociation, 
                        header.idMessage,
                        header.idReception,
                        header.type,
                        (byte*) donneesZones,
                        lengthRx
                    );
                    if(err != RADIOLIB_ERR_NONE) {
                        delay(30);
                        continue;
                    }

                    readBuffer = ReadBuffer(donneesZones, lengthRx);

                    struct {
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
                    } infosZones;
                    
                    if(readBuffer.remainingLength() < sizeof(header)) { return false; }
                    readBuffer.getBytes((byte *)&header, sizeof(header));

                    if(readBuffer.remainingLength() < sizeof(uint8_t)) { return false; }
                    uint8_t longueur = readBuffer.getUInt8();
                    
                    if(longueur != sizeof(infosZones)) {
                        return false;
                    }
                    if(readBuffer.remainingLength() < sizeof(infosZones)) { return false; }
                    readBuffer.getBytes((byte *)&infosZones, sizeof(infosZones));

                    delay(300);

                    if(! boostActif()) {
                        return true;
                    }

                    donneesZone.temperatureConsigne = getTemperatureBoost();

                    uint8_t retry = 0;
                    lengthRx = 0;
                    do {
                        delay(50);
                        err = this->radio().sendInit(
                            this->getId(), 
                            ID_CHAUDIERE, 
                            this->getIdAssociation(),
                            this->incrementIdMessage(),
                            0x01, 
                            0xA154,
                            0x0018,
                            0xA02F,
                            0x0004,
                            (byte*)&donneesZone,
                            sizeof(donneesZone),
                            donneesZones,
                            lengthRx
                        );

                        if(err != RADIOLIB_ERR_NONE) {
                            delay(100);
                            continue;
                        }
                        
                        return true;
                    } while(retry++ < 10);


                    return true;
                } while (retry++ < 5);
            }
        }
    } 

    return false;
}