#pragma once

#include "FrisquetDevice.h"
#include <Preferences.h>
#include "Logs.h"
#include "../DS18B20.h"

class SondeExterieure : public FrisquetDevice {
    
    public:
        SondeExterieure(FrisquetRadio& radio, Config& cfg, MqttManager& mqtt) : FrisquetDevice(radio, cfg, mqtt, ID_SONDE_EXTERIEURE) {}
        void loadConfig();
        void saveConfig();

        bool envoyerTemperatureExterieure();

        float getTemperatureExterieure();
        void setTemperatureExterieure(float temperature);

        void loop();
        void begin();
        void publishMqtt();

        void setDS18B20(DS18B20* ds18b20) { _ds18b20 = ds18b20; };

    private:
        float _temperatureExterieure = NAN;

        DS18B20* _ds18b20 = nullptr;
        uint32_t _lastEnvoiTemperatureExterieure = 0;

        // MQTT

        struct {
            MqttEntity tempExterieure;
        } _mqttEntities; 
};