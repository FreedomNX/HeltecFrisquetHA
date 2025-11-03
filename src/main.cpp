#include "config.h"
#include "radio.h"
#include "mqtt.h"
#include "utils.h"

#include <WiFi.h>
#include <Preferences.h>
#include <heltec.h>

#include <ArduinoOTA.h>

#include "Portal.h"

#include "devices/Connect.h"
#include "devices/SondeExterieure.h"
#include "devices/Satellite.h"

#if USE_DS18B20
#include "DS18B20.h"
#endif

#include "Timer.h"

#include "types/Date.h"

#include <TimeLib.h>

#include "Logs.h"

Radio *radio;
MQTT *mqtt;


Preferences preferences;

unsigned long lastTxExtSonTime = 30000;        // Variable dernière transmission sonde ajout de 30 sec d'intervale pour ne pas envoyé en meme temps que le connect
const unsigned long txExtSonInterval = 600000; // Interval de transmission en millisecondes (10 minutes)

Timer temperatureExterieureTimer = Timer(600000); // 10 minutes
Timer connectTimer = Timer(120000); // 2 minutes
Timer verificationBoostTimer = Timer(600000); // 10 minutes
Timer recuperationDateTimer = Timer(300000); // 5 minutes

bool associationMode = false;

uint8_t sondeExterieureAssociationId = 0xFF;
uint8_t frisquetConnectAssociationId = 0xFF;
uint8_t satelliteZ1AssociationId = 0xFF;
NetworkID networkId;

Connect *connect;
SondeExterieure *sondeExterieure;
Satellite* satelliteZ1;
float temperatureExterieure = 0;
bool temperatureExterieureAvailable = false;

#if USE_DS18B20
DS18B20 *ds18b20;
#endif

volatile bool radioDataAvailable = false;
Date date;

void initNvs() {
  // Démarre l'instance de NVS
  preferences.begin("net-conf", false);

  NetworkID defaultNetworkId = DEFAULT_NETWORK_ID;
  sondeExterieureAssociationId = (DEFAULT_SONDE_EXTERIEURE_ASSOCIATION_ID == 0xFF ? preferences.getUChar("son_id", 0) : DEFAULT_SONDE_EXTERIEURE_ASSOCIATION_ID);
  frisquetConnectAssociationId = (DEFAULT_FRISQUET_CONNECT_ASSOCIATION_ID == 0xFF ? preferences.getUChar("con_id", 0) : DEFAULT_FRISQUET_CONNECT_ASSOCIATION_ID);
  satelliteZ1AssociationId = (DEFAULT_SATELLITE_Z1_ASSOCIATION_ID == 0xFF ? preferences.getUChar("satz1_id", 0) : DEFAULT_SATELLITE_Z1_ASSOCIATION_ID);

  if ((preferences.getBytes("net_id", &networkId, sizeof(NetworkID)) != sizeof(NetworkID) || networkId.toInt32() == 0 || networkId.isBroadcast())
   && defaultNetworkId.toInt32() != 0) {
    preferences.putBytes("net_id", &defaultNetworkId, sizeof(NetworkID));
    networkId = defaultNetworkId;
  }

  preferences.end(); // Ferme la mémoire NVS
}

void eraseNvs() {
  preferences.begin("net-conf", false); // Ouvrir la mémoire NVS en mode lecture/écriture
  preferences.clear();                  // Effacer complètement la mémoire NVS
  preferences.end(); // Ferme la mémoire NVS
}


void sendPayload(byte byteArr[], size_t len) {  
    const char hexDigits[] = "0123456789ABCDEF";
    char message[255];
    message[0] = '\0';
    int pos = 0;

    // Convertir chaque octet en deux caractères hexadécimaux
    for (int i = 0; i < len; i++) {
        uint8_t b = byteArr[i];
        // Premier nibble (4 bits)
        message[pos++] = hexDigits[b >> 4];
        // Deuxième nibble
        message[pos++] = hexDigits[b & 0x0F];
        // Ajouter un espace séparateur
        message[pos++] = ' ';
        // Vérifier qu'on ne dépasse pas la taille du buffer
        if (pos >= (int)(sizeof(message) - 2))
        break;
    }

    // Si on a ajouté un espace en trop à la fin, on l'enlève
    if (pos > 0 && message[pos - 1] == ' ') {
        pos--;
    }

    // Terminer la chaîne
    message[pos] = '\0';
    mqtt->publish("homeassistant/sensor/frisquet/payload/state", message);
}


void initOTA() {
    ArduinoOTA.setHostname("ESP32Frisquet");
    ArduinoOTA.setTimeout(25000); // Augmenter le délai d'attente à 25 secondes
    ArduinoOTA
      .onStart([]()
               {
    String type;

    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    DBG_PRINTLN("OTA Start updating " + type); })
      .onEnd([]()
             { DBG_PRINTLN(F("\nEnd")); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
    DBG_PRINTF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DBG_PRINTLN(F("OTA Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) DBG_PRINTLN(F("OTA Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) DBG_PRINTLN(F("OTA Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) DBG_PRINTLN(F("OTA Receive Failed"));
    else if (error == OTA_END_ERROR) DBG_PRINTLN(F("OTA End Failed")); });

  ArduinoOTA.begin();
}


time_t syncDateFromConnect() {
  #if USE_CONNECT
    if(connect->recupererDate(&date)) {
      setTime(date.heure, date.minute, date.seconde, date.jour, date.mois, date.annee);
    }
  #endif

  return 0;
}


void onReceiveMQTT(String topic, byte *payload, unsigned int length) {
  String message = String(payload, length);
  message.replace(",", ".");

  DBG_PRINTLN(topic);
  DBG_PRINTLN(message);

  #if USE_CONNECT
  if(connect->isReady()) {
    Zone *zone1 = connect->getZone1();
    #if USE_ZONE_2
    Zone *zone2 = connect->getZone2();
    #endif
    #if USE_ZONE_3
    Zone *zone3 = connect->getZone3();
    #endif

    if(topic == MQTT_TEMP_CONSIGNE_CONFORT1_SET) {
      zone1->setTemperatureConfort(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_TEMP_CONSIGNE_REDUIT1_SET) {
      zone1->setTemperatureReduit(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_TEMP_CONSIGNE_HORSGEL1_SET) {
      zone1->setTemperatureHorsGel(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } if(topic == MQTT_MODE1_SET) {
      if(zone1->getNomMode().equalsIgnoreCase(message)) {
        return;
      }
      zone1->setMode(message);
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_MODE1) {
      zone1->setMode(message.c_str());
    } else if(topic == MQTT_TEMP_CONSIGNE_CONFORT1) {
      zone1->setTemperatureConfort(message.toFloat(), true);
    } else if(topic == MQTT_TEMP_CONSIGNE_REDUIT1) {
      zone1->setTemperatureReduit(message.toFloat(), true);
    } else if(topic == MQTT_TEMP_CONSIGNE_HORSGEL1) {
      zone1->setTemperatureHorsGel(message.toFloat(), true);
    }

    #if USE_ZONE_2
    if(topic == MQTT_TEMP_CONSIGNE_CONFORT2_SET) {
      zone2->setTemperatureConfort(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_TEMP_CONSIGNE_REDUIT2_SET) {
      zone2->setTemperatureReduit(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_TEMP_CONSIGNE_HORSGEL2_SET) {
      zone2->setTemperatureHorsGel(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_MODE2_SET) {
      if(zone2->getNomMode() == message) {
        return;
      }
      zone2->setMode(message.c_str());
      syncDateFromConnect();
      connect->envoyerZone(zone2);
    } else if(topic == MQTT_MODE2) {
      zone2->setMode(message.c_str());
    }
    #endif

    #if USE_ZONE_3
    if(topic == MQTT_TEMP_CONSIGNE_CONFORT3_SET) {
      zone3->setTemperatureConfort(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_TEMP_CONSIGNE_REDUIT3_SET) {
      zone3->setTemperatureReduit(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_TEMP_CONSIGNE_HORSGEL3_SET) {
      zone3->setTemperatureHorsGel(message.toFloat());
      syncDateFromConnect();
      connect->envoyerZone(zone1);
    } else if(topic == MQTT_MODE3_SET) {
      if(zone3->getNomMode() == message) {
        return;
      }
      zone3->setMode(message.c_str());
      connect->envoyerZone(zone3);
    } else if(topic == MQTT_MODE3) {
      zone3->setMode(message.c_str());
    }
    #endif
    
    if (topic == MQTT_HA_STATUS && message.equalsIgnoreCase("online")) {
      mqtt->connectTopics();
      mqtt->publish(MQTT_TEMP_EXTERIEURE, connect->getTemperatureExterieure());
      mqtt->publish(MQTT_TEMP_CDC, connect->getTemperatureCDC());
      mqtt->publish(MQTT_TEMP_ECS, connect->getTemperatureECS());

      mqtt->publish(MQTT_TEMP_CONSIGNE1, connect->getTemperatureConsigneZ1());
      mqtt->publish(MQTT_TEMP_AMBIANTE1, connect->getTemperatureAmbianteZ1());
      mqtt->publish(MQTT_TEMP_DEPART1, connect->getTemperatureDepartZ1());
      mqtt->publish(MQTT_MODE1, zone1->getMode());
      
      #if USE_ZONE_2
      mqtt->publish(MQTT_TEMP_AMBIANTE2, connect->getTemperatureAmbianteZ2());
      mqtt->publish(MQTT_TEMP_CONSIGNE2, connect->getTemperatureConsigneZ2());
      mqtt->publish(MQTT_TEMP_DEPART2, connect->getTemperatureDepartZ2());
      mqtt->publish(MQTT_MODE2, zone2->getMode());
      #endif

      #if USE_ZONE_3
      mqtt->publish(MQTT_TEMP_AMBIANTE3, connect->getTemperatureAmbianteZ3());
      mqtt->publish(MQTT_TEMP_CONSIGNE3, connect->getTemperatureConsigneZ3());
      mqtt->publish(MQTT_TEMP_DEPART3, connect->getTemperatureDepartZ3());
      mqtt->publish(MQTT_MODE3, zone3->getMode());
      #endif

      // TODO Voir pour vérifier jour actuel
      mqtt->publish(MQTT_CONSO_GAZ_CHAUFFAGE, connect->getConsommationGazChauffage());
      mqtt->publish(MQTT_CONSO_GAZ_ECS, connect->getConsommationGazECS());
    } else if(topic == MQTT_BOOST1_SET) {
      if(message == "ON") {
        if(connect->activerBoost(zone1)) {
          mqtt->publish(MQTT_BOOST1, "ON");
        }
      } else {
        if(connect->desactiverBoost(zone1)) {
          mqtt->publish(MQTT_BOOST1, "OFF");
        }
      }
    }
  }
  if(topic == MQTT_ASS_CON_SET && message.equalsIgnoreCase("ON")) {
    ASSOCIATION_INFOS association = Connect::associer(radio);
    if(! association.networkId.isBroadcast()) {
      networkId = association.networkId;
      frisquetConnectAssociationId = association.associationId;

      // Démarre l'instance de NVS
        preferences.begin("net-conf", false);
        preferences.putBytes("net_id", &networkId, sizeof(NetworkID));
        preferences.putUChar("con_id", frisquetConnectAssociationId);
        preferences.end(); // Ferme la mémoire NVS

        ESP.restart();
    }
    mqtt->publish(MQTT_ASS_CON, "OFF");
  }
  #endif

  #if USE_SONDE_EXTERIEURE
    if(topic == MQTT_TEMPERATURE_EXTERIEURE_SET) {
      temperatureExterieure = message.toFloat();
      temperatureExterieureAvailable = true;
    } else if(topic == MQTT_ASS_SON_SET && message.equalsIgnoreCase("ON")) {
    ASSOCIATION_INFOS association = SondeExterieure::associer(radio);
    if(! association.networkId.isBroadcast()) {
      networkId = association.networkId;
      sondeExterieureAssociationId = association.associationId;

      // Démarre l'instance de NVS
        preferences.begin("net-conf", false);
        preferences.putBytes("net_id", &networkId, sizeof(NetworkID));
        preferences.putUChar("son_id", sondeExterieureAssociationId); 
        preferences.end(); // Ferme la mémoire NVS

        ESP.restart();
    }
    mqtt->publish(MQTT_ASS_SON, "OFF");
  }
  #endif
}

void onReceiveRadio() {
  int16_t state;
  for (int i = 0; i < 3; ++i) {
    static byte donnees[RADIOLIB_SX126X_MAX_PACKET_LENGTH];
    state = radio->readData(donnees, 0);
    if (state == RADIOLIB_ERR_NONE) {
  
      int len = radio->getPacketLength();
      
      #if DEBUG
      sendPayload(donnees, len);
      #endif
      
      // Traitement des données
      if(len >= 4) {
        #if USE_CONNECT
        if (connect->isReady() != 0) { // Si Frisquet Connect connecté
            if (donnees[0] == ID_FRISQUET_CONNECT && donnees[1] == ID_CHAUDIERE && donnees[2] == frisquetConnectAssociationId) {
              DBG_PRINTLN("Réception données Connect");
              connect->setRollingCode(donnees[3]);
              if(connect->onReceive(&donnees[4], len-4)) {
                
                // Mis à jour des données
                // TODO Voir pour mettre dans une fonction publishZone ?
                Zone* zone1 = connect->getZone1();
                mqtt->publish(MQTT_TEMP_CONSIGNE_CONFORT1, zone1->getTemperatureConfort(), true);
                mqtt->publish(MQTT_TEMP_CONSIGNE_REDUIT1, zone1->getTemperatureReduit(), true);
                mqtt->publish(MQTT_TEMP_CONSIGNE_HORSGEL1, zone1->getTemperatureHorsGel(), true);

                if(zone1->getMode() != 0) {
                  mqtt->publish(MQTT_MODE1, zone1->getNomMode().c_str(), true);
                }

                mqtt->publish(MQTT_BOOST1, zone1->boostActif() ? "ON":"OFF", true);

                #if USE_ZONE_2
                // Mis à jour des données
                Zone* zone2 = connect->getZone2();
                mqtt->publish(MQTT_TEMP_CONSIGNE_CONFORT2, zone2->getTemperatureConfort(), true);
                mqtt->publish(MQTT_TEMP_CONSIGNE_REDUIT2, zone2->getTemperatureReduit(), true);
                mqtt->publish(MQTT_TEMP_CONSIGNE_HORSGEL2, zone2->getTemperatureHorsGel(), true);

                if(zone2->getMode() != 0) {
                  mqtt->publish(MQTT_MODE2, zone2->getNomMode().c_str(), true);
                }
                #endif

                #if USE_ZONE_3
                // Mis à jour des données
                Zone* zone3 = connect->getZone3();
                mqtt->publish(MQTT_TEMP_CONSIGNE_CONFORT3, zone3->getTemperatureConfort(), true);
                mqtt->publish(MQTT_TEMP_CONSIGNE_REDUIT3, zone3->getTemperatureReduit(), true);
                mqtt->publish(MQTT_TEMP_CONSIGNE_HORSGEL3, zone3->getTemperatureHorsGel(), true);

                if(zone3->getMode() != 0) {
                  mqtt->publish(MQTT_MODE3, zone3->getNomMode().c_str(), true);
                }
                #endif

              }
            }
        }
        #endif

        if(satelliteZ1->isReady()) {
          // Récéption chaudière vers Satellite (inverse possible également)
          if (donnees[0] == ID_ZONE_1 && donnees[1] == ID_CHAUDIERE && donnees[2] == satelliteZ1AssociationId) { 
            if(satelliteZ1->onReceive(&donnees[4], len-4)) {
              satelliteZ1->setRollingCode(donnees[3]);
            }
          }
        }
      }
      // Fin traitement
    
    }
  }
  radio->startReceive();
}

void setup() {
    Serial.begin(115200);
    DBG_PRINTLN(F("Démarrage..."));

    // INIT AFFICHAGE OLED
    Heltec.begin(false /*DisplayEnable disable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

    // INIT NVS
    initNvs();

    // WiFi + PORTAIL WEB
    portalInit();

    // INIT OTA
    initOTA();

    // INIT MQTT
    mqtt = new MQTT();
    mqtt->init();
    mqtt->setCallback(onReceiveMQTT);

    // INIT RADIO
    radio = new Radio(networkId);
    radio->init();
    radio->onReceive([]() {
      radioDataAvailable = true;
    });
    
    DBG_PRINTLN("NetworkID : " + byteArrayToHexString(networkId.bytes, 4));
    DBG_PRINT(F("[SX1262] Début de l'écoute radio... "));
    if(radio->startReceive() != RADIOLIB_ERR_NONE) {
        DBG_PRINT(F("Erreur lors de l'écoute."));
    }

    #if USE_DS18B20
      ds18b20 = new DS18B20(DS18B20_PIN);
      ds18b20->setOffset(DS18B20_OFFSET);
      if(ds18b20->isReady()) {
        DBG_PRINTLN(F("[DS18B20] OK, prêt."));
      }
    #endif

    #if USE_CONNECT
    connect = new Connect(radio, mqtt, frisquetConnectAssociationId);
    #endif

    #if USE_SONDE_EXTERIEURE
    sondeExterieure = new SondeExterieure(radio, mqtt, sondeExterieureAssociationId);
    #endif

    satelliteZ1 = new Satellite(radio, mqtt, ID_ZONE_1, satelliteZ1AssociationId);

    temperatureExterieureTimer.start();
    connectTimer.start(millis() + 120000);
    verificationBoostTimer.start();
    recuperationDateTimer.start();

    syncDateFromConnect();
}


void loop() {
  portalLoop();
  mqtt->loop();
  ArduinoOTA.handle();

  if(radioDataAvailable) {
    radioDataAvailable = false;
    onReceiveRadio();
  }

  #if USE_SONDE_EXTERIEURE
  //TODO Voir pour déplacer dans SondeExterieure:loop()
  if (temperatureExterieureTimer.tick()) { // Vérifier si 10 minutes se sont écoulées depuis la dernière transmission
    DBG_PRINT(F("[] Envoi température extérieure..."));
    #if USE_DS18B20
    if(ds18b20->getTemperature(&temperatureExterieure)) {
      temperatureExterieureAvailable = true;
      if(sondeExterieure->envoyerTemperature(temperatureExterieure)) {
        mqtt->publish(MQTT_TEMP_EXTERIEURE, temperatureExterieure);
        DBG_PRINTLN(F("OK"));
      }
    }
    #else
      if(temperatureExterieureAvailable && sondeExterieure->envoyerTemperature(temperatureExterieure)) {
          DBG_PRINTLN(F("OK"));
      }
    #endif
  }
  #endif

  #if USE_CONNECT



  if(recuperationDateTimer.tick()) {
    syncDateFromConnect();
  }

  //TODO Voir pour déplacer dans Connect:loop()
  if (connectTimer.tick()) { // Vérifier si 2 minutes se sont écoulées depuis la dernière transmission
    DBG_PRINT(F("[] Récupération des informations de la chaudière..."));

    if(connect->recupererTemperatures()) {
      DBG_PRINTLN(F("Récupérations températures OK"));
      mqtt->publish(MQTT_TEMP_AMBIANTE1, connect->getTemperatureAmbianteZ1());
      mqtt->publish(MQTT_TEMP_CONSIGNE1, connect->getTemperatureConsigneZ1());
      mqtt->publish(MQTT_TEMP_DEPART1, connect->getTemperatureDepartZ1());
      #if USE_ZONE_2
      mqtt->publish(MQTT_TEMP_AMBIANTE2, connect->getTemperatureAmbianteZ2());
      mqtt->publish(MQTT_TEMP_CONSIGNE2, connect->getTemperatureConsigneZ2());
      #endif
      #if USE_ZONE_3
      mqtt->publish(MQTT_TEMP_AMBIANTE3, connect->getTemperatureAmbianteZ3());
      mqtt->publish(MQTT_TEMP_CONSIGNE3, connect->getTemperatureConsigneZ3());
      #endif

      if(connect->getTemperatureExterieure() <= 85.0f) { // Evite les valeurs délirantes lorsque aucune sonde n'est paramétrée
        mqtt->publish(MQTT_TEMP_EXTERIEURE, connect->getTemperatureExterieure());
      }
      mqtt->publish(MQTT_TEMP_CDC, connect->getTemperatureCDC());
      mqtt->publish(MQTT_TEMP_ECS, connect->getTemperatureECS());
    }

    if(connect->recupererConsommationGaz()) {
      DBG_PRINTLN(F("Récupérations consommation gaz OK"));
      // TODO Voir pour vérifier jour actuel
      mqtt->publish(MQTT_CONSO_GAZ_CHAUFFAGE, connect->getConsommationGazChauffage());
      mqtt->publish(MQTT_CONSO_GAZ_ECS, connect->getConsommationGazECS());
    }
  }

  if (verificationBoostTimer.tick()) { // Vérifier si 10 minutes se sont écoulées depuis la dernière vérification
    connect->verifierBoost();
  }

  if(connect->envoiModeZ1().isStarted() && connect->envoiModeZ1().tick()) {
      connect->envoyerZone(connect->getZone1());
  }
  if(connect->envoiModeZ2().isStarted() && connect->envoiModeZ2().tick()) {
      connect->envoyerZone(connect->getZone2());
  }
  if(connect->envoiModeZ3().isStarted() && connect->envoiModeZ2().tick()) {
      connect->envoyerZone(connect->getZone3());
  }

  #endif

}