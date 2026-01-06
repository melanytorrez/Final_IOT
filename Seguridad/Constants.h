#ifndef CONSTANTS_H
#define CONSTANTS_H

// =================================================================
// ||                    GLOBAL CONFIGURATION                     ||
// =================================================================

// --- HARDWARE PINS ---
// Lighting
#define PIN_LED_BEDROOM    21 
#define PIN_LED_KITCHEN    16 

// Servo Motors (Doors)
#define PIN_SERVO_MAIN_DOOR    25 // Main entrance (RFID controlled)
#define PIN_SERVO_BEDROOM_DOOR 26 
#define PIN_SERVO_KITCHEN_DOOR 27 

// RFID Module (RC522)
#define PIN_RFID_SS    5
#define PIN_RFID_RST   22

// --- AWS IOT CONFIGURATION ---
const char* MQTT_BROKER_ENDPOINT = "a30eisqpnafvzg-ats.iot.us-east-1.amazonaws.com";
const char* AWS_THING_NAME       = "CentralDeSeguridad";
const int   MQTT_PORT            = 8883;

// --- MQTT TOPICS ---
// Shadow Updates
const char* TOPIC_SHADOW_UPDATE       = "$aws/things/CentralDeSeguridad/shadow/update";
const char* TOPIC_SHADOW_UPDATE_DELTA = "$aws/things/CentralDeSeguridad/shadow/update/delta";

// Persistence / Shadow Recovery
const char* TOPIC_SHADOW_GET          = "$aws/things/CentralDeSeguridad/shadow/get";
const char* TOPIC_SHADOW_GET_ACCEPTED = "$aws/things/CentralDeSeguridad/shadow/get/accepted";

// Custom RFID Topics
const char* TOPIC_RFID_REQUEST  = "CentralDeSeguridad/rfid/checkRequest";
const char* TOPIC_RFID_RESPONSE = "CentralDeSeguridad/rfid/checkResponse";

// --- WIFI MANAGER ---
const char* AP_NAME = "CentralSeguridad_AP";

#endif
