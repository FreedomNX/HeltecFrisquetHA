#include "FrisquetDevice.h"

bool FrisquetDevice::associer(NetworkID& networkId, uint8_t& idAssociation) {

    byte buff[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    size_t buffLength = 0;
    uint16_t err;
    FrisquetRadio::RadioTrameHeader* radioTrameHeader = (FrisquetRadio::RadioTrameHeader*)&buff;
    
    radio().setNetworkID({0xFF, 0xFF, 0xFF, 0xFF}); // Broadcast

    uint8_t retry = 0;
    unsigned long timeout = millis() + 30000;
    do {
        err = radio().receive(buff, 0);
        if(err != RADIOLIB_ERR_NONE) {
            continue;
        }
        buffLength = radio().getPacketLength();

        if(radioTrameHeader->idExpediteur == ID_CHAUDIERE && radioTrameHeader->type == FrisquetRadio::MessageType::ASSOCIATION) {
            FrisquetRadio::RadioTrameHeader radioTrameHeaderAnswer;
            radioTrameHeader->answer(radioTrameHeaderAnswer);
            memcpy(buff, &radioTrameHeaderAnswer, sizeof(radioTrameHeaderAnswer));

            err = radio().transmit(buff, buffLength);
            if (err != RADIOLIB_ERR_NONE) {
                continue;
            }

            idAssociation = radioTrameHeader->idAssociation;
            networkId = buff[6];
            
            radio().setNetworkID(networkId);
            return true;
        }
    } while(millis() < timeout);

    return false;
}

bool FrisquetDevice::recupererDate() {
    if(! estAssocie()) {
        return false;
    }

    byte buff[6] = {0};
    
    size_t length;
    uint16_t err;

    uint8_t retry = 0;
    do {
        length = sizeof(buff);
        err = this->radio().sendAsk(
            this->getId(), 
            ID_CHAUDIERE, 
            this->getIdAssociation(),
            this->incrementIdMessage(),
            0x01,
            0x0A2B,
            0x0004,
            buff,
            length
        );

        if(err != RADIOLIB_ERR_NONE) {
            delay(100);
            continue;
        }

        _date = buff;
        
        return true;
    } while(retry++ < 10);

    return false;
}