#include "SondeExterieure.h"

SondeExterieure::SondeExterieure(Radio *radio, MQTT *mqtt, uint8_t idAssociation) : Device(radio, mqtt, ID_SONDE_EXTERIEURE, idAssociation) {
}

bool SondeExterieure::envoyerTemperature(float temperature) {
    if(! this->isReady()) {
        return false;
    }

    TEMPERATURE_EXTERIEURE_TRAME donnees;
    donnees.temperatureExterieure = temperature;

    byte requete[sizeof(TEMPERATURE_EXTERIEURE_TRAME)];
    memcpy(requete, &donnees, sizeof(TEMPERATURE_EXTERIEURE_TRAME));
    
    uint8_t retry = 0;
    do {
        Serial.println("Envoi température");
        this->incrementRollingCode();
        int16_t err = this->sendData(ID_CHAUDIERE, requete, sizeof(requete));
        if(err != RADIOLIB_ERR_NONE) {
            delay(1000);
            continue;
        }

        byte donnees[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
        size_t length = 0;
        do {
            err = this->waitingAnswer(ID_CHAUDIERE, donnees, &length);
        } while(err == RADIOLIB_ERR_INVALID_PAYLOAD);

        if(err != RADIOLIB_ERR_NONE) {
            continue;
        }

        return true;
    } while(retry++ < 10);

    return false;
}

ASSOCIATION_INFOS SondeExterieure::associer(Radio *radio) {
    return Device::associer(radio, ID_SONDE_EXTERIEURE);
}