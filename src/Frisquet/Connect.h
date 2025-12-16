#pragma once

#include "FrisquetDevice.h"
#include <Preferences.h>
#include "../Logs.h"
#include "Zone.h"

class Connect : public FrisquetDevice {
    
    public:
        Connect(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt, Zone& zone1, Zone& zone2, Zone& zone3) : 
            FrisquetDevice(radio, cfg, mqtt, ID_CONNECT),
            _zone1(zone1),
            _zone2(zone2),
            _zone3(zone3) {}
        void loadConfig();
        void saveConfig();

        void begin();
        void loop();

         enum MODE_ECS : uint8_t {
                    INCONNU = 0XFF,
            STOP = 0x29,
            MAX = 0x01,
            ECO = 0x09,
            ECO_HORAIRES = 0x11,
            ECOPLUS = 0x19,
            ECOPLUS_HORAIRES = 0x21
        };


        Zone& getZone1() { return _zone1; }
        Zone& getZone2() { return _zone2; }
        Zone& getZone3() { return _zone3; }
        Zone& getZone(uint8_t idZone) { 
            switch (idZone) {
                default:
                case ID_ZONE_1: return _zone1;
                case ID_ZONE_2: return _zone2;
                case ID_ZONE_3: return _zone3;
            }
        }

        bool envoyerZone(Zone& zone);
        bool envoyerModeECS();
        bool recupererTemperatures();
        bool recupererConsommation();
        bool recupererModeECS();

        float getTemperatureExterieure();
        float getTemperatureECS();
        float getTemperatureCDC();

        int16_t getConsommationECS();
        int16_t getConsommationChauffage();

        MODE_ECS getModeECS();
        bool setModeECS(MODE_ECS modeECS);
        bool setModeECS(const String& modeECS);
        String getNomModeECS();

        bool onReceive(byte* donnees, size_t length);

        void publishMqtt();
    private:
        Zone& _zone1;
        Zone& _zone2;
        Zone& _zone3;

        float _temperatureECS = NAN;
        float _temperatureCDC = NAN;
        float _temperatureExterieure = NAN;
        
        int16_t _consommationGazECS = -1;
        int16_t _consommationGazChauffage = -1;

        MODE_ECS _modeECS = MODE_ECS::INCONNU;

        void setTemperatureExterieure(float temperature);
        void setTemperatureECS(float temperature);
        void setTemperatureCDC(float temperature);

        void setConsommationECS(int16_t consommation);
        void setConsommationChauffage(int16_t consommation);

        void envoiZone();

        bool _envoiZ1 = false;
        bool _envoiZ2 = false;
        bool _envoiZ3 = false;

        uint32_t _lastRecuperationTemperatures = 0;
        uint32_t _lastRecuperationModeECS = 0;
        uint32_t _lastRecuperationConsommation = 0;
        uint32_t _lastEnvoiZone = 0;

        // MQTT

        struct {    
            MqttEntity modeECS;
            MqttEntity tempECS;
            MqttEntity tempCDC;
            MqttEntity tempExterieure;
            MqttEntity consommationChauffage;
            MqttEntity consommationECS;
        } _mqttEntities;
};