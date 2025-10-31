#include "Satellite.h"
#include "../types/Zone.h"

Satellite::Satellite(Radio *radio, MQTT *mqtt, uint8_t idZone, uint8_t idAssociation) : Device(radio, mqtt, idZone, idAssociation) {
}

ASSOCIATION_INFOS Satellite::associer(Radio *radio, uint8_t idZone) {
    return Device::associer(radio, idZone);
}

void Satellite::setTemperatureConsigne(float temperature) { 
    this->temperatureConsigne = temperature;
}
float Satellite::getTemperatureConsigne() {
    return this->temperatureConsigne;
}

void Satellite::setTemperatureAmbiance(float temperature) {
    this->temperatureAmbiance = temperature;
}
float Satellite::getTemperatureAmbiance() {
    return this->temperatureAmbiance;
}

uint8_t Satellite::getIdZone() {
    return this->getIdExpediteur();
}

bool Satellite::onReceive(byte donnees[], size_t length) {
    if(length == sizeof(INFOS_ZONES_TRAME) && 
    donnees[0] == 0x01 && donnees[1] == 0x17 && donnees[2] == 0x2A) { // Récéption trame infos Zones
        INFOS_ZONES_TRAME trame;
        memcpy(&trame, donnees, sizeof(trame));
        DBG_PRINTF("Température extérieure : %f\n", trame.temperatureExterieure.toFloat()); // Test température extérieure
        mqtt->publish(MQTT_TEMP_EXTERIEURE,trame.temperatureExterieure.toFloat());

        if(this->getIdZone() == ID_ZONE_1) {
            this->setTemperatureConsigne(trame.temperatureConsigne1.toFloat());
            this->setTemperatureAmbiance(trame.temperatureAmbiante1.toFloat());
        } else if(this->getIdZone() == ID_ZONE_2) {
            this->setTemperatureConsigne(trame.temperatureConsigne2.toFloat());
            this->setTemperatureAmbiance(trame.temperatureAmbiante2.toFloat());
        } if(this->getIdZone() == ID_ZONE_3) {
            this->setTemperatureConsigne(trame.temperatureConsigne3.toFloat());
            this->setTemperatureAmbiance(trame.temperatureAmbiante3.toFloat());
        }

        return true;
    }
    
    return false;
}
