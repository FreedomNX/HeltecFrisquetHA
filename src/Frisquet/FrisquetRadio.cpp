#include "FrisquetRadio.h"
#include "../Buffer.h"
#include "../Logs.h"

bool FrisquetRadio::receivedFlag = false;
bool FrisquetRadio::interruptReceive = false;

uint16_t FrisquetRadio::sendAsk(
    uint8_t idExpediteur, 
    uint8_t idDestinataire, 
    uint8_t idAssociation, 
    uint8_t idMessage, 
    uint8_t idReception,
    fword adresseMemoire,
    fword tailleMemoire,
    byte* donneesReception,
    size_t& length
) {
    
    struct {
        RadioTrameHeader header;
        RadioTrameAsk body;
    } trame;

    trame.header.idExpediteur = idExpediteur;
    trame.header.idDestinataire = idDestinataire;
    trame.header.idAssociation = idAssociation;
    trame.header.idMessage = idMessage;
    trame.header.idReception = idReception;
    trame.header.type = FrisquetRadio::MessageType::READ;
    trame.body.adresseMemoire = adresseMemoire;
    trame.body.tailleMemoire = tailleMemoire;

    byte payload[sizeof(trame)];
    memcpy(payload, &trame, sizeof(payload));
    
    interruptReceive = true;
    uint8_t retry = 0;
    uint16_t err;
    do {
        delay(30);
        logRadio(false, (byte*)payload, sizeof(payload));
        err = this->transmit((byte*)&payload, sizeof(payload));
        if(err != RADIOLIB_ERR_NONE) {
            continue;
        }

        err = this->receiveExpected(idDestinataire, idExpediteur, idAssociation, idMessage, idReception|0x80, FrisquetRadio::MessageType::READ, donneesReception, length);
        if(err == RADIOLIB_ERR_NONE) {
            break;
        }
    } while (retry++ < 5);

    interruptReceive = false;
    startReceive();
    return err;
}

uint16_t FrisquetRadio::sendInit(
    uint8_t idExpediteur, 
    uint8_t idDestinataire, 
    uint8_t idAssociation, 
    uint8_t idMessage, 
    uint8_t idReception,
    fword adresseMemoireLecture,
    fword tailleMemoireLecture,
    fword adresseMemoireEcriture,
    fword tailleMemoireEcriture,
    byte* donneesEnvoi, 
    uint8_t longueurDonnees,
    byte* donneesReception, 
    size_t& length
) {

    struct {
        RadioTrameHeader header;
        RadioTrameInit body;
    } trame;

    trame.header.idExpediteur = idExpediteur;
    trame.header.idDestinataire = idDestinataire;
    trame.header.idAssociation = idAssociation;
    trame.header.idMessage = idMessage;
    trame.header.idReception = idReception;
    trame.header.type = FrisquetRadio::MessageType::INIT;
    trame.body.adresseMemoireLecture = adresseMemoireLecture;
    trame.body.tailleMemoireLecture = tailleMemoireLecture;
    trame.body.adresseMemoireEcriture = adresseMemoireEcriture;
    trame.body.tailleMemoireEcriture = tailleMemoireEcriture;
    trame.body.longueurDonneesEcriture = longueurDonnees;
    
    byte payload[sizeof(trame) + longueurDonnees];
    memcpy(payload, &trame, sizeof(trame));
    memcpy(&payload[sizeof(trame)], donneesEnvoi, longueurDonnees);

    interruptReceive = true;
    uint8_t retry = 0;
    uint16_t err;

    do {
        logRadio(false, (byte*)payload, sizeof(payload));
        err = this->transmit(payload, sizeof(payload));
        if(err != RADIOLIB_ERR_NONE) {
            continue;
        }
        
        err = this->receiveExpected(idDestinataire, idExpediteur, idAssociation, idMessage, idReception|0x80, FrisquetRadio::MessageType::INIT, donneesReception, length);
        if(err == RADIOLIB_ERR_NONE) {
            break;
        }
        delay(30);
    } while (retry++ < 5);

    interruptReceive = false;
    startReceive();
    return err;
}

uint16_t FrisquetRadio::sendAnswer(
    uint8_t idExpediteur, 
    uint8_t idDestinataire, 
    uint8_t idAssociation, 
    uint8_t idMessage, 
    uint8_t idReception,
    uint8_t type,
    byte* donneesEnvoi, 
    uint8_t longueurDonnees
) {

    FrisquetRadio::RadioTrameHeader header;

    header.idExpediteur = idExpediteur;
    header.idDestinataire = idDestinataire;
    header.idAssociation = idAssociation;
    header.idMessage = idMessage;
    header.idReception = idReception | 0x80;
    header.type = type;
    
    byte payload[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    WriteBuffer writeBuffer(payload);
    writeBuffer.putBytes((byte*)&header, sizeof(header));
    writeBuffer.putUInt8(longueurDonnees);
    writeBuffer.putBytes(donneesEnvoi, longueurDonnees);

    interruptReceive = true;
    uint8_t retry = 0;
    uint16_t err;
    do {
        delay(30);
        logRadio(false, (byte*)payload, writeBuffer.getLength());
        err = this->transmit(payload, writeBuffer.getLength());
        if(err != RADIOLIB_ERR_NONE) {
            delay(10);
            continue;
        }
        
        break;
    } while (retry++ < 5);

    interruptReceive = false;
    startReceive();
    return err;
}

uint16_t FrisquetRadio::receiveExpected(
    uint8_t idExpediteur, 
    uint8_t idDestinataire, 
    uint8_t idAssociation, 
    uint8_t idMessage, 
    uint8_t idReception,
    uint8_t type,
    byte* donnees, 
    size_t& length,
    uint8_t retry,
    bool useReadData
) {
    interruptReceive = true;

    byte buff[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    uint16_t err;
    RadioTrameHeader radioTrameHeader;

    do {
        if(useReadData) {
            err = this->readData(buff, 0);
        } else {
            err = this->receive(buff, 0);
        }

        if(err != RADIOLIB_ERR_NONE) {
            continue;
        }

        size_t len = this->getPacketLength();

        if(len < sizeof(radioTrameHeader)) {
            continue;
        }
        
        logRadio(true, (byte*)buff, len);

        memcpy(&radioTrameHeader, buff, sizeof(radioTrameHeader));
        if( radioTrameHeader.idExpediteur == idExpediteur && 
            radioTrameHeader.idDestinataire == idDestinataire && 
            radioTrameHeader.idMessage == idMessage &&
            radioTrameHeader.idReception ==  idReception &&
            radioTrameHeader.type ==  type
        ) {
            if(length == 0 || len < length) {
                length = len;   
            }
            memcpy(donnees, buff, length);
            break;
        } else {
            err = RADIOLIB_ERR_RX_TIMEOUT;
        }
    } while (retry-- > 0);


    interruptReceive = false;
    startReceive();
    return err;
}

void FrisquetRadio::setNetworkID(NetworkID networkID) {
    this->setSyncWord(networkID.bytes, sizeof(networkID.bytes));
}