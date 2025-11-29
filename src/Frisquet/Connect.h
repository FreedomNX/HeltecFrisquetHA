#pragma once

#include "FrisquetDevice.h"
#include <Preferences.h>
#include "../Logs.h"

class Connect : public FrisquetDevice {
    
    public:
        Connect(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt) : FrisquetDevice(radio, cfg, mqtt, ID_CONNECT) {}
        void loadConfig();
        void saveConfig();

        void begin();
        void loop();

        class Zone {
            public:
                enum MODE_ZONE : uint8_t {
                    INCONNU = 0x00,
                    AUTO = 0x05,
                    CONFORT = 0x06,
                    REDUIT = 0X07,
                    HORS_GEL = 0x08
                };

                struct Programmation {
                    byte dimanche[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    byte lundi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    byte mardi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    byte mercredi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    byte jeudi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    byte vendredi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    byte samedi[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                };
                
                Zone(uint8_t idZone) : _idZone(idZone) {};

                void setTemperatureConfort(float temperature);
                void setTemperatureReduit(float temperature);
                void setTemperatureHorsGel(float temperature);
                void setTemperatureAmbiante(float temperature);
                void setTemperatureConsigne(float temperature);
                void setTemperatureDepart(float temperature);
                void setTemperatureBoost(float temperature);

                float getTemperatureConfort();
                float getTemperatureReduit();
                float getTemperatureHorsGel();
                
                float getTemperatureConsigne();
                float getTemperatureAmbiante();
                float getTemperatureDepart();
                float getTemperatureBoost();

                uint8_t getIdZone();

                MODE_ZONE getMode();
                void setMode(MODE_ZONE mode);
                void setMode(const String& mode);
                byte getModeOptions();
                void setModeOptions(byte modeOptions);

                void activerBoost();
                void desactiverBoost();
                bool boostActif();

                String getNomMode();

                Programmation& getProgrammation() { return _programmation; };
                void setProgrammation(Programmation& programmation) { memcpy(&_programmation, &programmation, sizeof(programmation)); };

            private:
                uint8_t _idZone = 0x00;
                float _temperatureConfort = NAN;                       // Début 5°C -> max 30°C
                float _temperatureReduit = NAN;                        // Début 5°C -> max 30°C
                float _temperatureHorsGel = NAN;                       // Début 5°C -> max 30°C
                float _temperatureDepart = NAN;   
                float _temperatureConsigne = NAN; 
                float _temperatureAmbiante = NAN;
                float _temperatureBoost = NAN;

                MODE_ZONE _mode = MODE_ZONE::INCONNU;            // 0x05 auto - 0x06 confort - 0x07 reduit - 0x08 hors gel
                byte _modeOptions = 0x05;
                /*
                Mode Option structure bits
                    inconnu1: 1 bit,
                    boost: 1 bit,
                    inconnu2: 2 bits,
                    inconnu3: 2 bits,
                    derogation: 1 bit,
                    confort: 1 bit
                */

                Programmation _programmation;
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
        bool recupererTemperatures();
        bool recupererConsommation();
        bool recupererDate();

        float getTemperatureExterieure();
        float getTemperatureECS();
        float getTemperatureCDC();

        int16_t getConsommationECS();
        int16_t getConsommationChauffage();

        bool onReceive(byte* donnees, size_t length);

        void publishMqtt();
    private:
        Zone _zone1 = Zone(ID_ZONE_1);
        Zone _zone2 = Zone(ID_ZONE_2);
        Zone _zone3 = Zone(ID_ZONE_3);

        float _temperatureECS = NAN;
        float _temperatureCDC = NAN;
        float _temperatureExterieure = NAN;
        
        int16_t _consommationGazECS = -1;
        int16_t _consommationGazChauffage = -1;

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
        uint32_t _lastRecuperationConsommation = 0;
        uint32_t _lastEnvoiZone = 0;

        // MQTT

        struct {
            MqttEntity associationConnect;
        
            MqttEntity modeZ1;
            MqttEntity modeZ2;
            MqttEntity modeZ3;

            MqttEntity tempAmbianteZ1;
            MqttEntity tempAmbianteZ2;
            MqttEntity tempAmbianteZ3;

            MqttEntity tempConsigneZ1;
            MqttEntity tempConsigneZ2;
            MqttEntity tempConsigneZ3;

            MqttEntity tempConfortZ1;
            MqttEntity tempConfortZ2;
            MqttEntity tempConfortZ3;

            MqttEntity tempReduitZ1;
            MqttEntity tempReduitZ2;
            MqttEntity tempReduitZ3;

            MqttEntity tempHorsGelZ1;
            MqttEntity tempHorsGelZ2;
            MqttEntity tempHorsGelZ3;

            MqttEntity tempDepartZ1;
            MqttEntity tempDepartZ2;
            MqttEntity tempDepartZ3;

            MqttEntity tempBoostZ1;
            MqttEntity tempBoostZ2;
            MqttEntity tempBoostZ3;

            MqttEntity boostZ1;
            MqttEntity boostZ2;
            MqttEntity boostZ3;

            MqttEntity tempECS;
            MqttEntity tempCDC;
            MqttEntity tempExterieure;
            MqttEntity consommationChauffage;
            MqttEntity consommationECS;
        } _mqttEntities;
};