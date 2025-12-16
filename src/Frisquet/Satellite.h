#pragma once

#include "FrisquetDevice.h"
#include <Preferences.h>
#include "Logs.h"
#include "Zone.h"

class Satellite : public FrisquetDevice {
    
    public:
        enum MODE : uint8_t {
            REDUIT_PERMANENT = 0x00,
            CONFORT_PERMANENT = 0x01,
            REDUIT_DEROGATION = 0x02,
            CONFORT_DEROGATION = 0x03,
            REDUIT_AUTO = 0x04,
            CONFORT_AUTO = 0x05,
            HORS_GEL = 0x10,
            INCONNU = 0xFF,
        };

        Satellite(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt, Zone& zone) : FrisquetDevice(radio, cfg, mqtt, zone.getIdZone()), _zone(zone) {}
        void loadConfig();
        void saveConfig();

        void loop();
        void begin(bool modeVirtuel = false);
        void publishMqtt();

        bool envoyerConsigne();

        bool onReceive(byte* donnees, size_t length);

        MODE getMode();
        void setMode(MODE mode);
        String getNomMode();

        void setEcrasement(bool ecrasement) { _ecrasement = ecrasement; }
        bool getEcrasement() { return _ecrasement; }

        uint8_t getNumeroZone() {
            switch(this->getId()) {
                case ID_ZONE_1: 
                    return 1;
                case ID_ZONE_2: 
                    return 2;
                case ID_ZONE_3: 
                    return 3;
            }
            
            return 0;
        }

    private:
        Zone& _zone;

        uint32_t _lastEnvoiConsigne = 0;
        bool _modeVirtuel = false;
        bool _ecrasement = false;

        struct {
            MqttEntity ecrasementConsigne;
        } _mqttEntities;
};