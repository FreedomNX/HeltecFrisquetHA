#include "FrisquetManager.h"

FrisquetManager::FrisquetManager(FrisquetRadio &radio, Config &cfg, MqttManager &mqtt)
    :   _radio(radio), _cfg(cfg), _mqtt(mqtt), _sondeExterieure(radio, cfg, mqtt), _connect(radio, cfg, mqtt),
        _satelliteZ1(radio, cfg, mqtt, ID_ZONE_1), _satelliteZ2(radio, cfg, mqtt, ID_ZONE_2), _satelliteZ3(radio, cfg, mqtt, ID_ZONE_3)
{
}

void FrisquetManager::begin()
{
    // Init radio
    _radio.init();
    _radio.setNetworkID(_cfg.getNetworkID());

    initMqtt();

    if (_cfg.useConnect()) {
        _connect.begin();
        _connect.recupererDate();
    }

    if (_cfg.useSondeExterieure()) {
        _sondeExterieure.begin();
        _sondeExterieure.recupererDate();

        if (_cfg.useDS18B20()) {
            initDS18B20();
        }
    }
    if (_cfg.useSatelliteZ1()) {
        if(_cfg.useSatelliteVirtualZ1()) {
            _satelliteZ1.setModeVirtuel(true);
        }
        _satelliteZ1.begin();
    }
    if (_cfg.useSatelliteZ2()) {
        if(_cfg.useSatelliteVirtualZ2()) {
            _satelliteZ2.setModeVirtuel(true);
        }
        _satelliteZ2.begin();
    }
    if (_cfg.useSatelliteZ3()) {
        if(_cfg.useSatelliteVirtualZ3()) {
            _satelliteZ3.setModeVirtuel(true);
        }
        _satelliteZ3.begin();
    }

    _mqtt.publishAvailability(*_mqtt.getDevice("heltecFrisquet"), true);

    _radio.onReceive([]()
                     {
    if(!FrisquetRadio::interruptReceive) {
        FrisquetRadio::receivedFlag = true;
    } });
    _radio.startReceive();
}

void FrisquetManager::loop()
{
    uint32_t now = millis();

    if (FrisquetRadio::receivedFlag) { // Récéption données radio
        onRadioReceive();
    }

    if (_cfg.useConnect()) {
        _connect.loop();
    }
    
    if (_cfg.useSondeExterieure()) {
        _sondeExterieure.loop();
    }
    
    if (_cfg.useSatelliteZ1()) {
        _satelliteZ1.loop();
    }
}

void FrisquetManager::initDS18B20()
{
    info("[DS18B20] Initialisation du capteur de température.");
    _ds18b20 = new DS18B20();
    _ds18b20->init(DS18B20_PIN);

    if (_ds18b20->isReady())
    {
        float temperature;
        for (uint8_t i = 0; i < 10; i++)
        { // Boucle pour éviter les valeur incohérentes
            _ds18b20->getTemperature(temperature);
            delay(100);
        }
        info("[DS18B20] Capteur prêt.");

        _sondeExterieure.setDS18B20(_ds18b20);
    }
    else
    {
        error("[DS18B20] Impossible d'initialiser le capteur.");
    }
}

void FrisquetManager::initMqtt()
{

    info("[MQTT] Initialisation du device MQTT.");

    // Device commun
    _device.deviceId = "heltecFrisquet";
    _device.name = "Heltec Frisquet";
    _device.model = "Heltec Frisquet ";
    _device.manufacturer = "FreedomNX Lab";
    _device.baseTopic = _cfg.getMQTTOptions().baseTopic;
    _device.swVersion = "2.0.0";
    _mqtt.registerDevice(_device);
}

void FrisquetManager::onRadioReceive()
{
    FrisquetRadio::interruptReceive = true;
    FrisquetRadio::receivedFlag = false;

    byte buff[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    size_t length = 0;
    uint16_t err = _radio.readData(buff, 0);

    if (err != RADIOLIB_ERR_NONE)
    {
        FrisquetRadio::interruptReceive = false;
        return;
    }

    length = _radio.getPacketLength();
    _radio.startReceive();

    info("[RADIO] Réception données radio : %d bytes", length);

    logRadio(true, buff, length);

    if (length < sizeof(FrisquetRadio::RadioTrameHeader))
    {
        FrisquetRadio::interruptReceive = false;
        return;
    }

    FrisquetRadio::RadioTrameHeader *header = (FrisquetRadio::RadioTrameHeader *)buff;
    if (header->idDestinataire == _connect.getId() && _cfg.useConnect()) {
        info("[RADIO] Traitement données Connect");
        _connect.onReceive(buff, length);
    } else if (header->idDestinataire == _satelliteZ1.getId() && _cfg.useSatelliteZ1() && _cfg.useSatelliteVirtualZ1()) {
        info("[RADIO] Traitement données Satellite Z1");
        _satelliteZ1.onReceive(buff, length);
    } else if (header->idDestinataire == _satelliteZ2.getId() && _cfg.useSatelliteZ2() && _cfg.useSatelliteVirtualZ2()) {
        info("[RADIO] Traitement données Satellite Z2");
        _satelliteZ2.onReceive(buff, length);
    } else if (header->idDestinataire == _satelliteZ3.getId() && _cfg.useSatelliteZ3() && _cfg.useSatelliteVirtualZ3()) {
        info("[RADIO] Traitement données Satellite Z2");
        _satelliteZ3.onReceive(buff, length);
    } else if (header->idExpediteur == _satelliteZ1.getId() && _cfg.useSatelliteZ1() && !_cfg.useSatelliteVirtualZ1()) {
        info("[RADIO] Traitement données envoi Satellite Z1");
        _satelliteZ1.onReceive(buff, length);
    } else if (header->idExpediteur == _satelliteZ2.getId() && _cfg.useSatelliteZ2() && !_cfg.useSatelliteVirtualZ2()) {
        info("[RADIO] Traitement données envoi Satellite Z2");
        _satelliteZ2.onReceive(buff, length);
    } else if (header->idExpediteur == _satelliteZ3.getId() && _cfg.useSatelliteZ3() && !_cfg.useSatelliteVirtualZ3()) {
        info("[RADIO] Traitement données envoi Satellite Z3");
        _satelliteZ3.onReceive(buff, length);
    }

    FrisquetRadio::interruptReceive = false;
}