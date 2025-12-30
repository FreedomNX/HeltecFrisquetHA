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

        struct ETAT_CHAUDIERE {
            ETAT_CHAUDIERE() : fonctionnement(false), arretChauffage(false) {}
            ETAT_CHAUDIERE(byte etat) {
                set(etat);
            }
            void set(byte etat) {
                fonctionnement = (etat & 0b00001000) != 0;
                arretChauffage = (etat & 0b00000100) != 0;
            }
            byte toByte() {
                byte etat = 0;
                if(fonctionnement) etat |= 0b00001000;
                if(arretChauffage) etat |= 0b00000100;
                return etat;
            }

            bool fonctionnement = false;
            bool arretChauffage = false;

            String getLibelle() {
                if(arretChauffage) {
                    return "Arrêt chauffage";
                } else if(!fonctionnement) {
                    return "Veille";
                } else {
                    return "Fonctionnement";
                }
            }
        };

        Satellite(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt, Zone& zone) : FrisquetDevice(radio, cfg, mqtt, zone.getIdZone()), _zone(zone) {}
        void loadConfig();
        void saveConfig();

        void loop();
        void begin(bool modeVirtuel = false);
        void publishMqtt();

        bool envoyerConsigne();
        bool envoyerTemperatureAmbiante();
        bool recupererInfosChaudiere();

        bool onReceive(byte* donnees, size_t length);

        MODE getMode();
        void setMode(MODE mode);
        String getNomMode();

        void setEcrasement(bool ecrasement) { _ecrasement = ecrasement; }
        bool getEcrasement() { return _ecrasement; }

        void setEtatChaudiere(byte etatChaudiere) { _etatChaudiere.set(etatChaudiere); }
        ETAT_CHAUDIERE getEtatChaudiere() { return _etatChaudiere; }

        uint8_t getNumeroZone() {
            return _zone.getNumeroZone();
        }

    private:
        Zone& _zone;

        uint32_t _lastEnvoiConsigne = 0;
        bool _modeVirtuel = false;
        bool _ecrasement = false;
        ETAT_CHAUDIERE _etatChaudiere;

        struct {
            MqttEntity ecrasementConsigne;
            MqttEntity etatChaudiere;
        } _mqttEntities;
};