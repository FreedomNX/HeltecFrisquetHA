#pragma once
#include <heltec.h>


struct TEMPERATURE8 {
    uint8_t value = 0;

    TEMPERATURE8() {}
    TEMPERATURE8(uint8_t b0) {
        this->value = b0;
    }
    TEMPERATURE8(float value) {
        this->value = uint16_t((round(value * 2.0f) / 2)*10) - 50;   // Début 5°C -> 0 - MAX 30°C -> 250, incrément 0,5
    }

    float toFloat() const {
        return float((this->value+50) / 10.0f);
    }

    bool operator ==(const TEMPERATURE8 b) {
        return value == b.value;
    }
    bool operator ==(const float b) {
        return value == TEMPERATURE8(b).value;
    }
    bool operator !=(const TEMPERATURE8 b) {
        return value != b.value;
    }
    bool operator !=(const float b) {
        return value != TEMPERATURE8(b).value;
    }
    bool operator <=(const TEMPERATURE8 b) {
        return value <= b.value;
    }
    bool operator <=(const float b) {
        return value <= TEMPERATURE8(b).value;
    }
    bool operator <(const TEMPERATURE8 b) {
        return value <=b.value;
    }
    bool operator <(const float b) {
        return value < TEMPERATURE8(b).value;
    }
    bool operator >=(const TEMPERATURE8 b) {
        return value >= b.value;
    }
    bool operator >=(const float b) {
        return value >= TEMPERATURE8(b).value;
    }
    bool operator >(const TEMPERATURE8 b) {
        return value > b.value;
    }
    bool operator >(const float b) {
        return value > TEMPERATURE8(b).value;
    }
};

struct TEMPERATURE16 {
    byte bytes[2] = {0};

    TEMPERATURE16() {}
    TEMPERATURE16(byte b0, byte b1) {
        this->bytes[0] = b0;
        this->bytes[1] = b1;
    }
    TEMPERATURE16(byte bytes[]) {
        memcpy(this->bytes, bytes, 2);
    }
    TEMPERATURE16(float value) {
        int16_t intValue = static_cast<int16_t>(round(value * 10.0f));
        this->bytes[0] = (intValue >> 8) & 0xFF;
        this->bytes[1] = intValue & 0xFF;
    }

    float toFloat() const {
        int16_t intValue = static_cast<int16_t>((bytes[0] << 8) | bytes[1]);
        return intValue / 10.0f;
    }

    bool operator ==(const TEMPERATURE8 b) {
        return toFloat() == b.toFloat();
    }
    bool operator ==(const float b) {
        return toFloat() == b;
    }
    bool operator !=(const TEMPERATURE8 b) {
        return toFloat() != b.toFloat();
    }
    bool operator !=(const float b) {
        return toFloat() != b;
    }
    bool operator <=(const TEMPERATURE8 b) {
        return toFloat() <= b.toFloat();
    }
    bool operator <=(const float b) {
        return toFloat() <= b;
    }
    bool operator <(const TEMPERATURE8 b) {
        return toFloat() < b.toFloat();
    }
    bool operator <(const float b) {
        return toFloat() < b;
    }
    bool operator >=(const TEMPERATURE8 b) {
        return toFloat() >= b.toFloat();
    }
    bool operator >=(const float b) {
        return toFloat() >= b;
    }
    bool operator >(const TEMPERATURE8 b) {
        return toFloat() == b.toFloat();
    }
    bool operator >(const float b) {
        return toFloat() > b;
    }
};
