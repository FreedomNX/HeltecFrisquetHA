#include "DS18B20.h"

DS18B20::DS18B20() {}

bool DS18B20::init(uint8_t pin) {
    this->oneWire = OneWire(pin);
    this->ds18b20 = DallasTemperature(&this->oneWire);

    pinMode(pin, INPUT);
    ds18b20.begin();
    if (ds18b20.getDeviceCount() == 0) {
        return false;
    }

    if (!ds18b20.getAddress(dsAddr, 0)) {
        return false;
    }

    ds18b20.setResolution(dsAddr, 12); // 12 bits (0.0625°C)

    this->_isReady = true;
    return true;
}

bool DS18B20::isReady() {
    return this->_isReady;
}

bool DS18B20::getTemperature(float& temperature) {
    if(! this->isReady()) {
        return false;
    }

    ds18b20.requestTemperaturesByAddress(dsAddr);
    float t = ds18b20.getTempC(dsAddr);
    if (t == DEVICE_DISCONNECTED_C) {
        return false;
    }

    temperature = t + this->offset;
    return true;
}

void DS18B20::setOffset(uint8_t offset) {
    this->offset = offset;
}