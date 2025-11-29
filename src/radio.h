#pragma once

#include <heltec.h>
#include <RadioLib.h>

#define SS GPIO_NUM_8
#define RST_LoRa GPIO_NUM_12
#define BUSY_LoRa GPIO_NUM_13
#define DIO0 GPIO_NUM_14

class Radio {

    public:
        Radio();
        void init();

        int16_t startReceive() { return _radio.startReceive(); }
        int16_t receive(uint8_t data[], size_t len) { return _radio.receive(data, len); }
        int16_t readData(uint8_t data[], size_t len) { return _radio.readData(data, len); }
        int16_t transmit(byte data[], size_t len) { return _radio.transmit(data, len); }
        size_t getPacketLength() { return _radio.getPacketLength(); }
        void onReceive(void (*func)()) { _radio.setPacketReceivedAction(func);}
        void setSyncWord(uint8_t* syncWord, size_t len) { _radio.setSyncWord(syncWord, len); }

    private:
        SX1262 _radio;
};

