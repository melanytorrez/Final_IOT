// =================================================================
// ||        ESP32 #1: CentralDeSeguridad (OOP Version)           ||
// =================================================================
/*
 * Description: Controls Lights, Doors, and RFID for the Security Center.
 * Architecture: Object-Oriented with separate headers.
 * Platform: ESP32
 */

#include <ArduinoJson.h>
#include "Constants.h"
#include "SmartLight.h"
#include "SmartDoor.h"
#include "RfidScanner.h"
#include "AwsMqttManager.h"

// --- HARDWARE OBJECTS ---
SmartLight ledBedroom(PIN_LED_BEDROOM);
SmartLight ledKitchen(PIN_LED_KITCHEN);

SmartDoor doorMain(PIN_SERVO_MAIN_DOOR);
SmartDoor doorBedroom(PIN_SERVO_BEDROOM_DOOR);
SmartDoor doorKitchen(PIN_SERVO_KITCHEN_DOOR);

RfidScanner rfid(PIN_RFID_SS, PIN_RFID_RST);

// --- NETWORK MANAGER ---
AwsMqttManager mqttManager(AWS_THING_NAME);

// --- HELPER FUNCTIONS ---
void publishShadow() {
    StaticJsonDocument<512> doc;
    JsonObject reported = doc.createNestedObject("state").createNestedObject("reported");

    reported["luz_habitacion"]    = ledBedroom.getState();
    reported["luz_cocina"]        = ledKitchen.getState();
    reported["puerta_principal"]  = doorMain.getState();
    reported["puerta_habitacion"] = doorBedroom.getState();
    reported["puerta_cocina"]     = doorKitchen.getState();

    char buffer[512];
    serializeJson(doc, buffer);
    mqttManager.publish(TOPIC_SHADOW_UPDATE, buffer);
    Serial.println("[Shadow] State reported.");
}

void applyShadowChanges(JsonObject state) {
    bool changed = false;

    // --- LIGHTS ---
    if (state.containsKey("luz_habitacion")) {
        String val = state["luz_habitacion"];
        if (val != ledBedroom.getState()) { ledBedroom.setState(val); changed = true; }
    }
    if (state.containsKey("luz_cocina")) {
        String val = state["luz_cocina"];
        if (val != ledKitchen.getState()) { ledKitchen.setState(val); changed = true; }
    }

    // --- DOORS ---
    if (state.containsKey("puerta_habitacion")) {
        String val = state["puerta_habitacion"];
        if (val != doorBedroom.getState()) { doorBedroom.setState(val); changed = true; }
    }
    if (state.containsKey("puerta_cocina")) {
        String val = state["puerta_cocina"];
        if (val != doorKitchen.getState()) { doorKitchen.setState(val); changed = true; }
    }
    
    // NOTE: Main door is only updated logically to stay in sync, but not physically opened by shadow connection recovery
    // to prevent security risks. Only RFID or explicit voice command (if configured) opens it.
    if (state.containsKey("puerta_principal")) {
         // We might want to sync the state variable without moving the motor if needed
    }

    if (changed) {
        publishShadow();
    }
}

// --- MQTT CALLBACK ---
void onMessageReceived(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on: "); Serial.println(topic);
    String topicStr = String(topic);

    // 1. RFID Validation Response
    if (topicStr.equals(TOPIC_RFID_RESPONSE)) {
        StaticJsonDocument<256> doc;
        deserializeJson(doc, payload, length);
        String status = doc["status"];
        Serial.print("[RFID] Validation: "); Serial.println(status);

        if (status == "VALID") {
            Serial.println("[Access] GRANTED. Opening main door.");
            doorMain.openMomentarily(3000); // Open 3s then close
            publishShadow(); // Update state to cloud
        } else {
            Serial.println("[Access] DENIED.");
        }
        return;
    }

    // 2. Shadow Updates
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

    // Hardware Init
    doorMain.begin();
    doorBedroom.begin();
    doorKitchen.begin();
    rfid.begin();

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
        if (mqttManager.connected()) { // Just reconnected
            mqttManager.subscribe(TOPIC_SHADOW_UPDATE_DELTA);
            mqttManager.subscribe(TOPIC_RFID_RESPONSE);
            mqttManager.subscribe(TOPIC_SHADOW_GET_ACCEPTED);
            // Recover state
            mqttManager.publish(TOPIC_SHADOW_GET, "");
        }
    }

    // 2. RFID Check
    String uid;
    if (rfid.scanCard(uid)) {
        Serial.print("[RFID] Tag detected: "); Serial.println(uid);
        
        StaticJsonDocument<128> doc;
        doc["card_uid"] = uid;
        doc["thing_name"] = AWS_THING_NAME;

        char buffer[128];
        serializeJson(doc, buffer);
        mqttManager.publish(TOPIC_RFID_REQUEST, buffer);
        Serial.println("[RFID] Sending validation request...");
        delay(1000); // Prevent spamming
    }
}
