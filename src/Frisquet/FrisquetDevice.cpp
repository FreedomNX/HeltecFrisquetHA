#include "FrisquetDevice.h"
#include "../Buffer.h"

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

        ReadBuffer readBuffer = ReadBuffer(buff, buffLength);
        logRadio(true, (byte*)buff, sizeof(buffLength));

        if(radioTrameHeader->idExpediteur == ID_CHAUDIERE && radioTrameHeader->type == FrisquetRadio::MessageType::ASSOCIATION) {
            info("[DEVICE] Récéption trame d'association");

            struct {
                FrisquetRadio::RadioTrameHeader header;
                NetworkID networkID;
            } confirmPayload;
            
            radioTrameHeader->answer(confirmPayload.header);
            confirmPayload.header.idExpediteur = this->getId();
            confirmPayload.networkID = &buff[7];

            info("[DEVICE] Récupération du NetworkID : %s.", byteArrayToHexString((byte*)&confirmPayload.networkID, sizeof(NetworkID)).c_str());
            info("[DEVICE] Récupération de l'association ID : %s.", byteArrayToHexString((byte*)&radioTrameHeader->idAssociation, 1).c_str());

            err = radio().transmit((byte*)&confirmPayload, sizeof(confirmPayload));
            if (err != RADIOLIB_ERR_NONE) {
                continue;
            }

            idAssociation = radioTrameHeader->idAssociation;
            networkId = confirmPayload.networkID;
            
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