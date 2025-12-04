#pragma once
#include <heltec.h>
#include <Preferences.h>
#include "NetworkManager.h"
#include "MQTT/MqttManager.h"
#include "Frisquet/NetworkID.h"

class Config {
    private:
        NetworkManager::Options _wifiOpts;
        MqttManager::Options _mqttOpts;
        NetworkID _networkId;

        Preferences _preferences;

        bool _useConnect = false;
        bool _useSondeExterieure = false;
        bool _useDS18B20 = false;
        bool _useSatelliteZ1 = false;
    public:
        Config();
        void load();
        void save();

        NetworkManager::Options& getWiFiOptions() { return _wifiOpts; }
        MqttManager::Options& getMQTTOptions() { return _mqttOpts; }
        NetworkID& getNetworkID() { return _networkId; }
        void setNetworkID(NetworkID networkId) { _networkId = networkId; }

        bool useConnect() { return _useConnect; }
        bool useConnect(bool useConnect) { _useConnect = useConnect; return _useConnect; }
        bool useSondeExterieure() { return _useSondeExterieure; }
        bool useSondeExterieure(bool useSondeExterieure) { _useSondeExterieure = useSondeExterieure; return _useSondeExterieure; }
        bool useDS18B20() { return _useDS18B20; }
        bool useDS18B20(bool useDS18B20) { _useDS18B20 = useDS18B20; return _useDS18B20; }
        bool useSatelliteZ1() { return _useSatelliteZ1; }
        bool useSatelliteZ1(bool useSatelliteZ1) { _useSatelliteZ1 = useSatelliteZ1; return _useSatelliteZ1; }
};