#ifndef CONTROLSALA_CONSTANTS_H
#define CONTROLSALA_CONSTANTS_H

// =================================================================
// ||             SALA (LIVING ROOM) CONFIGURATION                ||
// =================================================================

// --- HARDWARE PINS ---
#define PIN_PIR_SENSOR  23
#define PIN_LED_SALA    12 

// Stepper Motor (28BYJ-48)
#define PIN_STEPPER_IN1 17
#define PIN_STEPPER_IN2 16
#define PIN_STEPPER_IN3 27
#define PIN_STEPPER_IN4 14

#define STEPS_PER_REV   2048

// --- AWS IOT CONFIGURATION ---
const char* MQTT_BROKER_ENDPOINT = "a30eisqpnafvzg-ats.iot.us-east-1.amazonaws.com";
const char* AWS_THING_NAME       = "ControlSala";
const int   MQTT_PORT            = 8883;

// --- MQTT TOPICS ---
// Shadow Updates
const char* TOPIC_SHADOW_UPDATE       = "$aws/things/ControlSala/shadow/update";
const char* TOPIC_SHADOW_UPDATE_DELTA = "$aws/things/ControlSala/shadow/update/delta";

// Persistence
const char* TOPIC_SHADOW_GET          = "$aws/things/ControlSala/shadow/get";
const char* TOPIC_SHADOW_GET_ACCEPTED = "$aws/things/ControlSala/shadow/get/accepted";

// --- WIFI MANAGER ---
const char* AP_NAME = "ControlSala_AP";

#endif
