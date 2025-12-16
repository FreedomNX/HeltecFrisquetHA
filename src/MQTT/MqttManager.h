#pragma once
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <vector>
#include <map>
#include <functional>
#include "MqttDevice.h"

class MqttManager {
public:
  struct Options {
    String clientId = "Heltec Frisquet";
    String host = "192.168.1.10";
    uint16_t port = 1883;
    String username;
    String password;
    String baseTopic = "frisquet"; // valeur par défaut
    uint16_t keepAliveSec = 60;
    bool cleanSession = true;
  };

  explicit MqttManager(Client& net) : _client(net) {}

  void begin(const Options& o) {
    _opts = o;
    _mqtt.setClient(_client);
    _mqtt.setServer(_opts.host.c_str(), _opts.port);
    _mqtt.setKeepAlive(_opts.keepAliveSec);
    _mqtt.setSocketTimeout(15);
    _mqtt.setBufferSize(_bufferSize);

    _mqtt.setCallback([this](char* topic, uint8_t* payload, unsigned int len) {
      String t(topic), p; p.reserve(len);
      for (unsigned int i=0;i<len;i++) p += (char)payload[i];
      auto it = _commandHandlers.find(t);
      if (it != _commandHandlers.end()) it->second(p);
    });
  }

  bool loop() { if (!_mqtt.connected()) reconnect(); return _mqtt.loop(); }
  bool connected() { return _mqtt.connected(); }

  // --- Device & Entity registration ---

  // Enregistre un device (injecte baseTopic/availability par défaut si manquant)
  void registerDevice(MqttDevice& d) {
    if (!d.baseTopic.length()) d.baseTopic = _opts.baseTopic;
    if (!d.availabilityTopic.full.length()) {
      d.availabilityTopic = MqttTopic(compose({d.baseTopic, d.deviceId, "availability"}), 1, true);
    }
    _devices[d.deviceId] = &d;
  }

  // Ajoute une entité au device et publie sa discovery (optionnel)
  void registerEntity(MqttDevice& d, MqttEntity& e, bool publishDiscovery = true) {
    if (!isRegistered(&d)) registerDevice(d);
    e.device = &d;
    d.entities[e.id] = &e;

    if (publishDiscovery) publishEntityDiscovery(e);

    // Abonnement commande si présent
    if (e.commandTopic.full.length()) _mqtt.subscribe(e.commandTopic.full.c_str());
  }

  // Command router
  using CommandCallback = std::function<void(const String&)>;
  bool onCommand(const MqttEntity& e, CommandCallback cb) {
    if (!e.commandTopic.full.length()) return false;
    _commandHandlers[e.commandTopic.full] = cb;
    return _mqtt.subscribe(e.commandTopic.full.c_str());
  }
  bool onCommand(const MqttTopic& commandTopic, CommandCallback cb) {
    if (!commandTopic.full.length()) return false;
    _commandHandlers[commandTopic.full] = cb;
    return _mqtt.subscribe(commandTopic.full.c_str());
  }

  // Publish helpers
  bool publishAvailability(const MqttDevice& d, bool online) {
    return publishRaw(d.availabilityTopic.full, online ? d.payloadAvailable : d.payloadNotAvailable, 1, true);
  }

  bool publishState(const MqttEntity& e, const String& payload) {
    if (!e.stateTopic.full.length()) return false;
    return publishRaw(e.stateTopic.full, payload, e.stateTopic.qos, e.stateTopic.retain);
  }

  bool publishState(const MqttEntity& e, float value, uint8_t decimals = 2) {
    char buf[32]; dtostrf(value, 0, decimals, buf);
    return publishState(e, String(buf));
  }

  bool publishJson(const MqttTopic& topic, const JsonDocument& doc) {
    if (!topic.full.length()) return false;
    char* buf = new char[_bufferSize];
    size_t n = serializeJson(doc, buf, _bufferSize);
    bool ok = (n > 0) && _mqtt.publish(topic.full.c_str(), (uint8_t*)buf, n, topic.retain);
    delete[] buf;
    return ok;
  }

  static String compose(std::initializer_list<String> parts, char sep = '/') {
    String out;
    for (auto& p : parts) { if (out.length()) out += sep; out += p; }
    return out;
  }

  MqttDevice* getDevice(String id) {
    auto it = _devices.find(id);
    
    if (it != _devices.end()) {
      return it->second;
    }
    
    return new MqttDevice();
  }

private:
  Client& _client;
  PubSubClient _mqtt;
  Options _opts;
  const size_t _bufferSize = 2048;

  std::map<String, MqttDevice*> _devices;
  std::map<String, CommandCallback> _commandHandlers;

  bool isRegistered(MqttDevice* d) const {
    for (auto i : _devices) if (i.second == d) return true;
    return false;
  }

  void reconnect() {
    if (_mqtt.connected()) return;
    if (_mqtt.connect(_opts.clientId.c_str(),
                      _opts.username.length()? _opts.username.c_str(): nullptr,
                      _opts.password.length()? _opts.password.c_str(): nullptr,
                      nullptr, 0, false, nullptr, _opts.cleanSession)) {

      // Birth de chaque device + (re)publication des discovery + resubscribe
      for (auto i : _devices) {
        MqttDevice *d = i.second;
        publishAvailability(*d, true);
        for (auto j : d->entities) {
          MqttEntity* e = j.second;
          publishEntityDiscovery(*e);
          if (e->commandTopic.full.length()) _mqtt.subscribe(e->commandTopic.full.c_str());
        }
      }
    }
  }

  bool publishRaw(const String& topic, const String& payload, uint8_t qos = 0, bool retain = true) {
    if (!topic.length()) return false;
    return _mqtt.publish(topic.c_str(), (uint8_t*)payload.c_str(), payload.length(), retain);
  }

  bool publishEntityDiscovery(const MqttEntity& e) {
    JsonDocument doc;
    e.buildDiscoveryJson(doc);
    char json[1024];
    size_t n = serializeJson(doc, json, sizeof(json));
    if (!n) return false;
    return publishRaw(e.discoveryTopic(), json, 0, true);
  }
};
