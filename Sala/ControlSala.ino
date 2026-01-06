// =================================================================
// ||           ESP32 #2: ControlSala (OOP Version)               ||
// =================================================================
/*
 * Description: Controls Living Room Light, Window (Stepper), and Motion Sensor.
 * Architecture: Object-Oriented with separate headers.
 * Platform: ESP32
 */

#include <ArduinoJson.h>
#include "ControlSala_Constants.h"
#include "SmartLight.h"
#include "SmartStepper.h"
#include "MotionSensor.h"
#include "AwsMqttManager.h"

// --- HARDWARE OBJECTS ---
SmartLight ledSala(PIN_LED_SALA);

// Stepper connections: IN1, IN3, IN2, IN4 (Standard for 28BYJ-48 lib sequence)
// But our class takes: Steps, IN1, IN3, IN2, IN4 to handle the library eccentricity if needed
// Let's pass them as defined in constants.
SmartStepper windowMotor(STEPS_PER_REV, PIN_STEPPER_IN1, PIN_STEPPER_IN3, PIN_STEPPER_IN2, PIN_STEPPER_IN4);

MotionSensor pirSensor(PIN_PIR_SENSOR);

// --- NETWORK MANAGER ---
AwsMqttManager mqttManager(AWS_THING_NAME);

// --- HELPER FUNCTIONS ---
void publishShadow() {
    StaticJsonDocument<512> doc;
    JsonObject reported = doc.createNestedObject("state").createNestedObject("reported");

    reported["luz_sala"]        = ledSala.getState();
    reported["ventana_sala"]    = windowMotor.getState();
    reported["movimiento_sala"] = pirSensor.getState();

    char buffer[512];
    serializeJson(doc, buffer);
    mqttManager.publish(TOPIC_SHADOW_UPDATE, buffer);
    Serial.println("[Shadow] State reported.");
}

void applyShadowChanges(JsonObject state) {
    bool changed = false;

    // --- LIGHT ---
    if (state.containsKey("luz_sala")) {
        String val = state["luz_sala"];
        if (val != ledSala.getState()) { ledSala.setState(val); changed = true; }
    }

    // --- WINDOW ---
    if (state.containsKey("ventana_sala")) {
        String val = state["ventana_sala"];
        if (val != windowMotor.getState()) { windowMotor.setState(val); changed = true; }
    }

    if (changed) {
        publishShadow();
    }
}

// --- MQTT CALLBACK ---
void onMessageReceived(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on: "); Serial.println(topic);
    String topicStr = String(topic);

    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) { Serial.println("JSON Error"); return; }

    if (topicStr.equals(TOPIC_SHADOW_UPDATE_DELTA)) {
        if (doc.containsKey("state")) {
            applyShadowChanges(doc["state"]);
        }
    } else if (topicStr.equals(TOPIC_SHADOW_GET_ACCEPTED)) {
        Serial.println("[Shadow] Recovering state...");
        if (doc.containsKey("state")) {
            JsonObject state = doc["state"];
            if (state.containsKey("desired")) applyShadowChanges(state["desired"]);
            else if (state.containsKey("reported")) applyShadowChanges(state["reported"]);
        }
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    // WiFi Init
    mqttManager.setupWiFi(AP_NAME);

    // AWS Init
    mqttManager.setupAWS(MQTT_BROKER_ENDPOINT, MQTT_PORT, onMessageReceived);
}

// --- LOOP ---
void loop() {
    // 1. Maintain Connection
    mqttManager.loop();
    if (!mqttManager.connected()) {
        if (mqttManager.connected()) { // Reconnected
            mqttManager.subscribe(TOPIC_SHADOW_UPDATE_DELTA);
            mqttManager.subscribe(TOPIC_SHADOW_GET_ACCEPTED);
            // Recover state
            mqttManager.publish(TOPIC_SHADOW_GET, "");
        }
    }

    // 2. Motion Sensor
    if (pirSensor.check()) {
        Serial.print("[Sensor] Motion: "); Serial.println(pirSensor.getState());
        publishShadow();
    }
}
