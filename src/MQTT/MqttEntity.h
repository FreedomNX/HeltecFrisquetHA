#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include "MqttTopic.h"

struct MqttDevice; // forward

struct MqttEntity {
  // Identité minimale
  String id;             // "tempAmbianteZ1"
  String name;           // "Température Z1"
  String component;      // "sensor", "select", ...
  String domain = "homeassistant";

  // Topics
  MqttTopic stateTopic;
  MqttTopic commandTopic;
  MqttTopic attributesTopic;
  MqttTopic availabilityTopic;

  // Availability payloads (entité)
  String payloadAvailable = "online";
  String payloadNotAvailable = "offline";

  // Champs dynamiques arbitraires
  std::map<String, String> extraFields;

  // Lien vers le device parent (renseigné automatiquement à l’enregistrement)
  const MqttDevice* device = nullptr;

  template<typename T>
  void set(const String& key, const T& value) { extraFields[key] = String(value); }

  // Valeur brute JSON (ex: tableaux: options, etc.)
  void setRaw(const String& key, const String& rawJson) { extraFields[key] = rawJson; }

  String discoveryTopic() const;                // défini après MqttDevice
  void buildDiscoveryJson(JsonDocument& doc) const;

private:
  static bool isNumber(const String& s) {
    if (!s.length()) return false;
    bool dot = false;
    for (size_t i = 0; i < s.length(); i++) {
      char c = s[i];
      if (c == '.') { if (dot) return false; dot = true; }
      else if (!isdigit(c) && c != '-') return false;
    }
    return true;
  }
};
