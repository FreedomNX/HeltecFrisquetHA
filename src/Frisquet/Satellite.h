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

        struct MODE_CHAUDIERE {
            MODE_CHAUDIERE() : fonctionnement(false), arretForce(false) {}
            MODE_CHAUDIERE(byte mode) {
                set(mode);
            }
            void set(byte mode) {
                fonctionnement = (mode & 0b001000) != 0;
                arretForce = (mode & 0b000100) != 0;
            }
            byte toByte() {
                byte mode = 0;
                if(fonctionnement) mode |= 0b001000;
                if(arretForce) mode |= 0b000100;
                return mode;
            }

            bool fonctionnement = false;
            bool arretForce = false;
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

        void setModeChaudiere(byte modeChaudiere) { _modeChaudiere.set(modeChaudiere); }
        MODE_CHAUDIERE getModeChaudiere() { return _modeChaudiere; }

        uint8_t getNumeroZone() {
            return _zone.getNumeroZone();
        }

    private:
        Zone& _zone;

        uint32_t _lastEnvoiConsigne = 0;
        bool _modeVirtuel = false;
        bool _ecrasement = false;
        MODE_CHAUDIERE _modeChaudiere;

        struct {
            MqttEntity ecrasementConsigne;
        } _mqttEntities;
};