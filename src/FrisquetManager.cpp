#include "FrisquetManager.h"

FrisquetManager::FrisquetManager(FrisquetRadio &radio, Config &cfg, MqttManager &mqtt)
    : _radio(radio), _cfg(cfg), _mqtt(mqtt), _sondeExterieure(radio, cfg, mqtt), _connect(radio, cfg, mqtt)
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
    }

    if (_cfg.useSondeExterieure()) {
        _sondeExterieure.begin();

        if (_cfg.useDS18B20()) {
            initDS18B20();
        }
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

    if (FrisquetRadio::receivedFlag)
    { // Récéption données radio
        onRadioReceive();
        _radio.startReceive();
    }

    if (_cfg.useConnect())
    {
        _connect.loop();
    }
    
    if (_cfg.useSondeExterieure())
    {
        _sondeExterieure.loop();
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

    info("[RADIO] Réception données radio");

    length = _radio.getPacketLength();

    logRadio(true, buff, length);

    if (length < sizeof(FrisquetRadio::RadioTrameHeader))
    {
        FrisquetRadio::interruptReceive = false;
        return;
    }

    FrisquetRadio::RadioTrameHeader *header = (FrisquetRadio::RadioTrameHeader *)buff;
    if (header->idDestinataire == _connect.getId() && _cfg.useConnect())
    {
        info("[RADIO] Traitement données Connect");
        _connect.onReceive(buff, length);
    }

    FrisquetRadio::interruptReceive = false;
}