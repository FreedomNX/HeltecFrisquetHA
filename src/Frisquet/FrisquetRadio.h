#pragma once
#include "../Radio.h"
#include "Utils.h"
#include "NetworkID.h"

class FrisquetRadio : public Radio {
    public: 

    enum MessageType : byte {
        READ = 0x03,
        INIT = 0x17,
        ASSOCIATION = 0x41
    };
    
    struct RadioTrameHeader {
        byte idDestinataire = 0x00;
        byte idExpediteur = 0x00;
        byte idAssociation = 0x00;
        byte idMessage = 0x00;
        byte idReception = 0x01; //0x01 => Si message direct, sinon id destinataire finale (exemple envoi au satellite Z1 via chaudière) + 0x80 si accusé récéption
        byte type = 0x00;

        void answer(RadioTrameHeader& radioTrameHeader) {
            radioTrameHeader.idExpediteur = idDestinataire;
            radioTrameHeader.idDestinataire = idExpediteur;
            radioTrameHeader.idAssociation = idAssociation;
            radioTrameHeader.idMessage = idMessage;
            radioTrameHeader.idReception = idReception | 0x80;
            radioTrameHeader.type = type;
        }

        bool isAck() {
            return idReception >= 0x80;
        }
    };

    struct RadioTrameAsk {
        fword adresseMemoire;
        fword tailleMemoire;
    };

    struct RadioTrameInit {
        fword adresseMemoireLecture;
        fword tailleMemoireLecture;
        fword adresseMemoireEcriture;
        fword tailleMemoireEcriture;
        uint8_t longueurDonneesEcriture;
    };

    uint16_t sendAsk(
        uint8_t idExpediteur, 
        uint8_t idDestinataire, 
        uint8_t idAssociation, 
        uint8_t idMessage, 
        uint8_t idReception,
        fword adresseMemoire,
        fword tailleMemoire,
        byte* donneesReception,
        size_t& length,
        uint8_t retry = 5
    );

    uint16_t sendInit(
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
    );

    uint16_t sendAnswer(
        uint8_t idExpediteur, 
        uint8_t idDestinataire, 
        uint8_t idAssociation, 
        uint8_t idMessage, 
        uint8_t idReception,
        uint8_t type,
        byte* donneesEnvoi, 
        uint8_t longueurDonnees
    );
    
    uint16_t receiveExpected(
            uint8_t idExpediteur, 
            uint8_t idDestinataire, 
            uint8_t idAssociation, 
            uint8_t idMessage, 
            uint8_t idReception, 
            uint8_t type,
            byte* donnees, 
            size_t& length,
            uint8_t retry = 5,
            bool useReadData = false
        );

    void setNetworkID(NetworkID networkID);

    static bool receivedFlag;
    static bool interruptReceive;
};