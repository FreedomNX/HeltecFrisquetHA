#pragma once

#include "FrisquetDevice.h"
#include <Preferences.h>
#include "Logs.h"

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
            BOOST = 0x0F,
            INCONNU = 0xFF,
        };

        Satellite(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt, uint8_t idZone) : FrisquetDevice(radio, cfg, mqtt, idZone) {}
        void loadConfig();
        void saveConfig();

        void loop();
        void begin();
        void publishMqtt();

        bool envoyerConsigne();

        bool onReceive(byte* donnees, size_t length);

        MODE getMode() { return _mode; }
        void setMode(MODE mode) { _mode = mode; }
        void setMode(const String& mode) {
            if (mode.equalsIgnoreCase("Auto")) {
                this->setMode(MODE::CONFORT_AUTO);
            } else if (mode.equalsIgnoreCase("Réduit")) {
                this->setMode(MODE::REDUIT_PERMANENT);
            } else if (mode.equalsIgnoreCase("Hors gel")) {
                this->setMode(MODE::HORS_GEL);
            } else if (mode.equalsIgnoreCase("Confort")) {
                this->setMode(MODE::CONFORT_PERMANENT);
            }
        }
        String getNomMode();

        void setTemperatureAmbiante(float temperature);
        void setTemperatureConsigne(float temperature);
        float getTemperatureConsigne() { return this->_temperatureConsigne; }
        float getTemperatureAmbiante() { return this->_temperatureAmbiante; }


        void setTemperatureConfort(float temperature);
        void setTemperatureReduit(float temperature);
        void setTemperatureHorsGel(float temperature);
        void setTemperatureBoost(float temperature);

        float getTemperatureConfort() { return _temperatureConfort; };
        float getTemperatureReduit() { return _temperatureReduit; };
        float getTemperatureHorsGel() { return _temperatureHorsGel; };

        bool boostActif() { return _boost; }
        void activerBoost() { _boost = true; }
        void desactiverBoost() { _boost = false; }
        float getTemperatureBoost() { return _temperatureBoost; }

        void setModeVirtuel(bool modeVirtuel) { _modeVirtuel = modeVirtuel; }
        void setActif(bool actif) { _actif = actif; }
        bool actif() { return _actif; }

        bool confortActif();
        bool reduitActif();
        bool horsGelActif();
        bool derogationActive();
        bool autoActif();

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

        float _temperatureAmbiante = NAN;
        float _temperatureConsigne = NAN;
        float _temperatureConfort = NAN;
        float _temperatureReduit = NAN;
        float _temperatureHorsGel = NAN;
        float _temperatureBoost = 5;

        MODE _mode = MODE::INCONNU;

        bool _modeVirtuel = false;
        bool _boost = false; 
        bool _actif = false;
        uint32_t _lastEnvoiConsigne = 0;

        struct {
            MqttEntity actif;
            MqttEntity temperatureAmbiante;
            MqttEntity temperatureConsigne;
            MqttEntity temperatureBoost;
            MqttEntity temperatureConfort;
            MqttEntity temperatureReduit;
            MqttEntity temperatureHorsGel;
            MqttEntity boost;
            MqttEntity mode;
            MqttEntity thermostat;
        } _mqttEntities;
        
};