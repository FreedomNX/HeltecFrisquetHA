#pragma once

#include <ArduinoOTA.h>
#include "Logs.h"

class OTA {
    public: 
        void begin(const char* hostname) {
            ArduinoOTA.setHostname(hostname);
            ArduinoOTA.setTimeout(25000);
            ArduinoOTA
                .onStart([]() {
                info("Mise à jour via OTA...");
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                    type = "sketch";
                else // U_SPIFFS
                    type = "filesystem";
            }).onEnd([](){ 
            }).onProgress([](unsigned int progress, unsigned int total){ 
            }).onError([](ota_error_t err) {
                error("Erreur lors de la mise à jour !");
            });

            ArduinoOTA.begin();
        }

        void loop() {
            ArduinoOTA.handle();
        }
};