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
        String getNomMode();

        void setTemperatureAmbiante(float temperature) { this->_temperatureAmbiante = temperature; }
        void setTemperatureConsigne(float temperature) { this->_temperatureConsigne = std::min(30.0f, std::max(5.0f, temperature)); }
        float getTemperatureConsigne() { return this->_temperatureConsigne; }
        float getTemperatureAmbiante() { return this->_temperatureAmbiante; }

        bool boostActif() { return _boost; }
        void activerBoost() { _boost = true; }
        void desactiverBoost() { _boost = false; }
        void setTemperatureBoost(float temperature) { _temperatureBoost = std::min(5.0f, std::max(0.0f, temperature)); }
        float getTemperatureBoost() { return _temperatureBoost; }

        void setModeVirtuel(bool modeVirtuel) { _modeVirtuel = modeVirtuel; }

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
        MODE _mode = MODE::INCONNU;

        bool _modeVirtuel = false;

        bool _boost = false;
        float _temperatureBoost = 5;

        uint32_t _lastEnvoiConsigne = 0;

        struct {
            MqttEntity temperatureAmbiante;
            MqttEntity temperatureConsigne;
            MqttEntity temperatureBoost;
            MqttEntity boost;
            MqttEntity mode;
            MqttEntity thermostat;
        } _mqttEntities;
        
};