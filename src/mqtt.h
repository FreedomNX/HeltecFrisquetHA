#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include <WiFi.h>


// Constantes pour les topics MQTT ( A SUPPRIMER ?)
#define MQTT_TEMP_EXTERIEURE "homeassistant/sensor/frisquet/tempExterieure/state"

#define MQTT_TEMP_AMBIANTE1 "homeassistant/sensor/frisquet/tempAmbiante1/state"
#define MQTT_TEMP_CONSIGNE1 "homeassistant/sensor/frisquet/tempConsigne1/state"
#define MQTT_TEMP_CONSIGNE1_SET "homeassistant/sensor/frisquet/tempConsigne1/set"
#define MQTT_TEMP_CONSIGNE_CONFORT1 "homeassistant/number/frisquet/tempConsigneConfort1/state"
#define MQTT_TEMP_CONSIGNE_REDUIT1 "homeassistant/number/frisquet/tempConsigneReduit1/state"
#define MQTT_TEMP_CONSIGNE_HORSGEL1 "homeassistant/number/frisquet/tempConsigneHorsGel1/state"
#define MQTT_TEMP_CONSIGNE_CONFORT1_SET "homeassistant/number/frisquet/tempConsigneConfort1/set"
#define MQTT_TEMP_CONSIGNE_REDUIT1_SET "homeassistant/number/frisquet/tempConsigneReduit1/set"
#define MQTT_TEMP_CONSIGNE_HORSGEL1_SET "homeassistant/number/frisquet/tempConsigneHorsGel1/set"
#define MQTT_TEMP_DEPART1 "homeassistant/sensor/frisquet/tempDepart1/state"
#define MQTT_MODE1 "homeassistant/select/frisquet/mode1/state"
#define MQTT_MODE1_SET "homeassistant/select/frisquet/mode1/set"
#define MQTT_BOOST1 "homeassistant/switch/frisquet/boost1/state"
#define MQTT_BOOST1_SET "homeassistant/switch/frisquet/boost1/set"

#define MQTT_TEMP_AMBIANTE2 "homeassistant/sensor/frisquet/tempAmbiante2/state"
#define MQTT_TEMP_CONSIGNE2 "homeassistant/sensor/frisquet/tempConsigne2/state"
#define MQTT_TEMP_CONSIGNE_CONFORT2 "homeassistant/number/frisquet/tempConsigneConfort2/state"
#define MQTT_TEMP_CONSIGNE_REDUIT2 "homeassistant/number/frisquet/tempConsigneReduit2/state"
#define MQTT_TEMP_CONSIGNE_HORSGEL2 "homeassistant/number/frisquet/tempConsigneHorsGel2/state"
#define MQTT_TEMP_CONSIGNE_CONFORT2_SET "homeassistant/number/frisquet/tempConsigneConfort2/set"
#define MQTT_TEMP_CONSIGNE_REDUIT2_SET "homeassistant/number/frisquet/tempConsigneReduit2/set"
#define MQTT_TEMP_CONSIGNE_HORSGEL2_SET "homeassistant/number/frisquet/tempConsigneHorsGel2/set"
#define MQTT_TEMP_DEPART2 "homeassistant/sensor/frisquet/tempDepart2/state"
#define MQTT_MODE2 "homeassistant/select/frisquet/mode2/state"
#define MQTT_MODE2_SET "homeassistant/select/frisquet/mode2/set"

#define MQTT_TEMP_AMBIANTE3 "homeassistant/sensor/frisquet/tempAmbiante3/state"
#define MQTT_TEMP_CONSIGNE3 "homeassistant/sensor/frisquet/tempConsigne3/state"
#define MQTT_TEMP_CONSIGNE_CONFORT3 "homeassistant/number/frisquet/tempConsigneConfort3/state"
#define MQTT_TEMP_CONSIGNE_REDUIT3 "homeassistant/number/frisquet/tempConsigneReduit3/state"
#define MQTT_TEMP_CONSIGNE_HORSGEL3 "homeassistant/number/frisquet/tempConsigneHorsGel3/state"
#define MQTT_TEMP_CONSIGNE_CONFORT3_SET "homeassistant/number/frisquet/tempConsigneConfort3/set"
#define MQTT_TEMP_CONSIGNE_REDUIT3_SET "homeassistant/number/frisquet/tempConsigneReduit3/set"
#define MQTT_TEMP_CONSIGNE_HORSGEL3_SET "homeassistant/number/frisquet/tempConsigneHorsGel/set"
#define MQTT_TEMP_DEPART3 "homeassistant/sensor/frisquet/tempDepart3/state"
#define MQTT_MODE3 "homeassistant/select/frisquet/mode3/state"
#define MQTT_MODE3_SET "homeassistant/select/frisquet/mode3/set"

#define MQTT_TEMP_CDC "homeassistant/sensor/frisquet/tempCDC/state"
#define MQTT_TEMP_ECS "homeassistant/sensor/frisquet/tempECS/state"

#define MQTT_CONSO_GAZ_ECS "homeassistant/sensor/frisquet/consogaz-ecs/state"
#define MQTT_CONSO_GAZ_CHAUFFAGE "homeassistant/sensor/frisquet/consogaz-ch/state"

#define MQTT_ASS_SON "homeassistant/switch/frisquet/asssonde/state"
#define MQTT_ASS_SON_SET "homeassistant/switch/frisquet/asssonde/set"

#define MQTT_ASS_CON "homeassistant/switch/frisquet/assconnect/state"
#define MQTT_ASS_CON_SET "homeassistant/switch/frisquet/assconnect/set"

#define MQTT_RES_NVS "homeassistant/switch/frisquet/erasenvs/state"
#define MQTT_RES_NVS_SET "homeassistant/switch/frisquet/erasenvs/set"

#define MQTT_TEMPERATURE_EXTERIEURE "homeassistant/sensor/frisquet/tempExterieure/set"
#define MQTT_TEMPERATURE_EXTERIEURE_SET "homeassistant/sensor/frisquet/tempExterieure/set"

#define MQTT_HA_STATUS "homeassistant/status" //TODO Ajouter ce topic en configurable

class MQTT {
    public:
        MQTT();
        void init();
        void setCallback(MQTT_CALLBACK_SIGNATURE);
        boolean publish(const char *topic, const char *payload, bool retain = false);
        boolean publish(const char *topic, float value, bool retain = false);
        boolean loop();

        boolean connected();
        boolean connectToMQTT();
        void connectTopics();

    private:
        WiFiClient wifiClient;
        PubSubClient client;


        void connecterModeZ1();
        void connecterTemperatureAmbianteZ1();
        void connecterTemperatureConsigneZ1();
        void connecterTemperatureConsigneHorsGelZ1();
        void connecterTemperatureConsigneReduitZ1();
        void connecterTemperatureConsigneConfortZ1();
        void connecterBoostZ1();

        void connecterModeZ2();
        void connecterTemperatureAmbianteZ2();
        void connecterTemperatureConsigneZ2();
        void connecterTemperatureConsigneHorsGelZ2();
        void connecterTemperatureConsigneReduitZ2();
        void connecterTemperatureConsigneConfortZ2();

        void connecterModeZ3();
        void connecterTemperatureAmbianteZ3();
        void connecterTemperatureConsigneZ3();
        void connecterTemperatureConsigneHorsGelZ3();
        void connecterTemperatureConsigneReduitZ3();
        void connecterTemperatureConsigneConfortZ3();


        void connecterTemperatureExterieure();
        void connecterTemperatureCDC();
        void connecterTemperatureDepartZ1();
        void connecterTemperatureECS();

        void connecterAssociationSondeExterieure();
        void connecterAssociationConnect();

        void connecterConsoGazChauffage();
        void connecterConsoGazECS();

        void connecterEffacerNVS();

        void connecterPayload();



        void connectToTemperatureSensor(const char *topic, const char *name);
        void connectToSwitch(const char *topic, const char *name);
};


#endif