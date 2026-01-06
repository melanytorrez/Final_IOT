#ifndef AWS_MQTT_MANAGER_H
#define AWS_MQTT_MANAGER_H

#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "ControlSala_Certificates.h"

class AwsMqttManager {
private:
    WiFiClientSecure _net;
    PubSubClient _client;
    WiFiManager _wm;
    String _thingName;
    
    typedef void (*MqttCallback)(char*, byte*, unsigned int);

public:
    AwsMqttManager(const char* thingName) : _client(_net) {
        _thingName = thingName;
    }

    void setupWiFi(const char* apName) {
        Serial.println("\n[WiFi] Initializing WiFiManager...");
        // _wm.resetSettings(); // Uncomment to reset saved credentials
        
        bool res = _wm.autoConnect(apName);
        if (!res) {
            Serial.println("[WiFi] Failed to connect. Restarting...");
            ESP.restart();
        }
        Serial.println("[WiFi] Connected!");
        Serial.print("[WiFi] IP: "); Serial.println(WiFi.localIP());
    }

    void setupAWS(const char* broker, int port, MqttCallback callback) {
        _net.setCACert(AMAZON_ROOT_CA1);
        _net.setCertificate(DEVICE_CERTIFICATE);
        _net.setPrivateKey(DEVICE_PRIVATE_KEY);

        _client.setServer(broker, port);
        _client.setCallback(callback);
    }

    void connect() {
        Serial.print("[MQTT] Connecting to AWS as "); Serial.print(_thingName); Serial.print("... ");
        if (_client.connect(_thingName.c_str())) {
            Serial.println("Connected!");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(_client.state());
            Serial.println(" trying again in 5 seconds");
            delay(5000);
        }
    }

    bool connected() {
        return _client.connected();
    }

    void loop() {
        if (!connected()) {
            connect();
        }
        _client.loop();
    }

    void publish(const char* topic, const char* payload) {
        _client.publish(topic, payload);
    }

    void subscribe(const char* topic) {
        _client.subscribe(topic);
        Serial.print("[MQTT] Subscribed to: "); Serial.println(topic);
    }
};

#endif
