// =================================================================
// ||   CÓDIGO ESP32 #1: CentralDeSeguridad (WiFiManager + Sync)  ||
// =================================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiManager.h> // <-- REQUISITO: Librería para gestión de WiFi

// --- CONFIGURACIÓN DEL HARDWARE ---
#define LED_HAB_PIN        21 // LED habitación
#define LED_COCINA_PIN     16 // LED cocina

#define SERVO_PUERTA_PRINCIPAL_PIN   25 // Puerta principal (RFID)
#define SERVO_PUERTA_HAB_PIN         26 // Puerta habitación
#define SERVO_PUERTA_COCINA_PIN      27 // Puerta cocina

// RFID RC522
#define RFID_SS_PIN   5
#define RFID_RST_PIN  22

// --- CONFIGURACIÓN AWS ---
const char* MQTT_BROKER = "a30eisqpnafvzg-ats.iot.us-east-1.amazonaws.com";
const char* THING_NAME  = "CentralDeSeguridad";

// --- CERTIFICADOS AWS ---
const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUSkwDOmGpK6AQnwxVjK2FT7Eco1swDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTIxMzAzNDQ0
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALcGS9FfsSepK95OwMZe
1oJV4NbZOXZwfrdcVM8mZWRdWL2+57OnArmtsaeYdT7iy3L65/mi7hZWxXvb1I0b
g9fwI86iewkXqT2JxS988w185AO0IG1iF1/dC7dCB5Q4dWF3lz2EAYARqgDnI4U0
ERbZS3/7OgMV06oyMZVLVrnc3AEAg52Bci77QS0iyw45ye0kN497ahxSV6G+ziO+
PJg9A3oEV+uTDeXYPmH9bKF658upcQ1yKZgam2GYXxiv3MS6rsaQQP6OYhet4zny
aRClqqIBkuxiENUhPi3QFqV8g7KI35jJSqIdXc4XoB08+Im/WW0yxCxgE9OEDv1m
HgMCAwEAAaNgMF4wHwYDVR0jBBgwFoAURe/8rdEGeLpQ5GCbXCTgfUTc5IUwHQYD
VR0OBBYEFEWUgt+BRkMlD0hQujhV4OWUABaDMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAxHrnIQ7fZa4UsVAaluTy+AhzY
w4PcvihqzQGehTgyvkFrQr5/9SNR1qJAHsMt0LAxMokZqkMccCYBMLCs8sxVmPce
qtsI/F022LRTStCeDCb6SgfXOkc0dKw9FwXPISb3fSdnGI+/qSL8ge7LzH/uCTez
aeUO0HVJI7D8B4OCBpEVXuNkUujKB8K7G7Ido2rVh04gl+JTKfsjAFL0r0vrrYxC
Gi6ZQirXVWsB072cQDH8TxF7SwOhasSevfPGHaZcufAFKnSuY9OEcGDUB8GL3XmL
RjkCERw1mMo1Af1zrbGdx5M79Pb3V+uq2qCS6YqUKQ+izmUJFLAr02tWhISI
-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAtwZL0V+xJ6kr3k7Axl7WglXg1tk5dnB+t1xUzyZlZF1Yvb7n
s6cCua2xp5h1PuLLcvrn+aLuFlbFe9vUjRuD1/AjzqJ7CRepPYnFL3zzDXzkA7Qg
bWIXX90Lt0IHlDh1YXeXPYQBgBGqAOcjhTQRFtlLf/s6AxXTqjIxlUtWudzcAQCD
nYFyLvtBLSLLDjnJ7SQ3j3tqHFJXob7OI748mD0DegRX65MN5dg+Yf1soXrny6lx
DXIpmBqbYZhfGK/cxLquxpBA/o5iF63jOfJpEKWqogGS7GIQ1SE+LdAWpXyDsojf
mMlKoh1dzhegHTz4ib9ZbTLELGAT04QO/WYeAwIDAQABAoIBAQCJzuHTmpvHYye2
0dFxDAO0S3lqDFGqDnY7Ffh9qUl60pZ4+H950+zHZjN4H+FYImhSAP4eB0IB6//y
jMl0Hh2dwCEV+11ssTYrMcsZQJm5tCnzZ/NqqQs0kTm7GKqgPgqUAvoLxZNjnOlG
mMM0Dso3TBH/IWjC3fJsGjMfWITKmG1kY6APNGApM8qm0qqCl1g3t/tT4HXGcplw
XToojkFcCWTvhIL/ncike0xe0vKrj6Aa7sOUcEnL60jyVZnTPHap5kbuwPIaSMnu
cGqQmMXO40xYPUnyRdOvi2Dk3RvuQKxP+Ar+SRiwzFPlu9Fjvrf1B3WnDnx2rnt6
7iUsvg1BAoGBAOOsvj5SDaRZlzI9Y+Ya0ddV6wxutn8pnWHBAxqbQiqWgnJtwOhS
TPJERhtWq1XM5bdzzDc3Wx3B3NCcR1JkhDtAdJqTjcupyKT4ayfXs7uH90DwQJov
hRnfv7jUMwpJ5QRKHpzTC1QUGkK1AoHOgxCTiv22i3zYmxsNhOptExiTAoGBAM3L
ex3syt6055zfgt/RlPwBoA8am21VwGJ8YDdGU7LpbMkCgLOsUdIeKajOqvMNY2XD
zv+HZrsCTSoVhJ24htgnQCZp5dPJBxV5rudpvVJdQNF0MlNJ9ApdRx9wQ33J1KOL
DCCaft+qnNl99OgGKGtccICzQoXFyIJRIw15I3rRAoGAUSjJAGo7lrShIzWjIOm6
l+p2yY3F3Hq3MZueQaPK6GeSxLu/IpID//C6lRBGL2XFFapfx+chwe/TiUHMYuCW
CpElf9zNlSZG8hfkCuXnmhgT2cBLJyt9ZQXP0FFEiIxi6S6KzxikVyp7WgAwjplR
O119FIg4HR8R24jsMq0DO6UCgYAliYIReUmfybudf59tl9rnBJzfZGkJTmTjEfCy
5F8L+UvgdlZnb30VKG9M3TWNU9nXjBnLvjTc05Sjon0oD6p47t/iodpMARULs8BV
cZIY3e2exuUPECQ1Z8I7V8zXuuLQWeKCZ+vQfFxTx4fOcqEYxCm5L/loUSU7r/UG
1hvEoQKBgHbQZUB5YlxtphXtnj+ohFuxsAXYrrFcEuEJm/B1Tw0znLnGxUUe6cLs
nNj0JC5lxJm+i7ERIvEaQgfE+Ho3Cq9cLTujMhxh+aOu5kAE7n2eHZQcSAmCYg5L
Bs8mYq3/5hIh7AToKCbjH0eUe99V3JltvUsnjGEKcG1pT+EE/Nu/
-----END RSA PRIVATE KEY-----
)KEY";

// --- TEMAS MQTT ---
const char* UPDATE_TOPIC            = "$aws/things/CentralDeSeguridad/shadow/update";
const char* UPDATE_DELTA_TOPIC      = "$aws/things/CentralDeSeguridad/shadow/update/delta";

// REQUISITO: Temas para recuperar estado
const char* GET_SHADOW_TOPIC        = "$aws/things/CentralDeSeguridad/shadow/get";
const char* GET_SHADOW_ACCEPTED     = "$aws/things/CentralDeSeguridad/shadow/get/accepted";

// Temas RFID
const char* RFID_CHECK_REQUEST_TOPIC  = "CentralDeSeguridad/rfid/checkRequest";
const char* RFID_CHECK_RESPONSE_TOPIC = "CentralDeSeguridad/rfid/checkResponse";

// --- OBJETOS GLOBALES ---
WiFiClientSecure wiFiClient;
PubSubClient mqttClient(wiFiClient);
Servo servoPuertaPrincipal, servoPuertaHab, servoPuertaCocina;
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
WiFiManager wm; // Instancia de WiFiManager

// --- ESTADOS LÓGICOS ---
String luzHabState         = "OFF";
String luzCocinaState      = "OFF";
String puertaPrincipalState = "CLOSED";
String puertaHabState       = "CLOSED";
String puertaCocinaState    = "CLOSED";

// --- PROTOTIPOS ---
void publishShadowUpdate();
void reconnectMQTT();
void setupWiFi();

// --- CONFIGURACIÓN WIFI CON WIFIMANAGER ---
void setupWiFi() {
  Serial.println("\nIniciando WiFiManager...");
  // wm.resetSettings(); // Descomentar para borrar credenciales guardadas
  
  // Nombre del AP: "CentralSeguridad_AP"
  bool res = wm.autoConnect("CentralSeguridad_AP"); 

  if(!res) {
    Serial.println("Fallo al conectar. Reiniciando...");
    ESP.restart();
  } 
  Serial.println("\nWiFi conectado exitosamente.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando MQTT como 'CentralDeSeguridad'...");
    if (mqttClient.connect(THING_NAME)) {
      Serial.println("Conectado a AWS IoT.");
      
      // Suscripciones
      mqttClient.subscribe(UPDATE_DELTA_TOPIC); 
      mqttClient.subscribe(RFID_CHECK_RESPONSE_TOPIC); 
      mqttClient.subscribe(GET_SHADOW_ACCEPTED); // Suscripción para recuperación
      
      Serial.println("Suscrito a delta, rfid response y get/accepted.");
      
      // REQUISITO: Solicitar estado previo
      mqttClient.publish(GET_SHADOW_TOPIC, "");
      Serial.println("Solicitando estado previo del Shadow...");
      
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Reintentando en 5s...");
      delay(5000);
    }
  }
}

// --- PUBLICACIÓN DE ESTADO ---
void publishShadowUpdate() {
  StaticJsonDocument<512> doc;
  JsonObject reported = doc.createNestedObject("state").createNestedObject("reported");

  reported["luz_habitacion"] = luzHabState;
  reported["luz_cocina"]     = luzCocinaState;
  reported["puerta_principal"]  = puertaPrincipalState;
  reported["puerta_habitacion"] = puertaHabState;
  reported["puerta_cocina"]     = puertaCocinaState;

  char buffer[512];
  serializeJson(doc, buffer);
  mqttClient.publish(UPDATE_TOPIC, buffer);
  Serial.println("Shadow de 'CentralDeSeguridad' reportado:");
  Serial.println(buffer);
}

// --- LÓGICA DE APLICACIÓN DE ESTADO (PERSISTENCIA) ---
void aplicarCambios(JsonObject state) {
  bool changed = false;

  // --- LUCES ---
  if (state.containsKey("luz_habitacion")) {
    String desired = state["luz_habitacion"];
    if (desired != luzHabState) {
      Serial.print("Aplicando Luz Habitacion: "); Serial.println(desired);
      digitalWrite(LED_HAB_PIN, desired == "ON" ? HIGH : LOW);
      luzHabState = desired;
      changed = true;
    }
  }
  if (state.containsKey("luz_cocina")) {
    String desired = state["luz_cocina"];
    if (desired != luzCocinaState) {
      Serial.print("Aplicando Luz Cocina: "); Serial.println(desired);
      digitalWrite(LED_COCINA_PIN, desired == "ON" ? HIGH : LOW);
      luzCocinaState = desired;
      changed = true;
    }
  }

  // --- PUERTAS (Controladas por Alexa/Cloud) ---
  if (state.containsKey("puerta_habitacion")) {
    String desired = state["puerta_habitacion"];
    if (desired != puertaHabState) {
      Serial.print("Aplicando Puerta Habitacion: "); Serial.println(desired);
      servoPuertaHab.write(desired == "OPEN" ? 90 : 0);
      puertaHabState = desired;
      changed = true;
    }
  }
  if (state.containsKey("puerta_cocina")) {
    String desired = state["puerta_cocina"];
    if (desired != puertaCocinaState) {
      Serial.print("Aplicando Puerta Cocina: "); Serial.println(desired);
      servoPuertaCocina.write(desired == "OPEN" ? 90 : 0);
      puertaCocinaState = desired;
      changed = true;
    }
  }

  // La puerta principal solo se activa por RFID localmente, no restauramos "OPEN"
  // por seguridad (evitar que se abra sola al reiniciar), pero sí sincronizamos el estado lógico.
  if (state.containsKey("puerta_principal")) {
     puertaPrincipalState = state["puerta_principal"].as<String>();
  }

  if (changed) {
    Serial.println("Estado sincronizado, reportando a AWS...");
    publishShadowUpdate();
  }
}

// --- CALLBACK MQTT ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje en: ");
  Serial.println(topic);
  String topicStr = String(topic);

  // 1. RESPUESTA RFID
  if (topicStr.equals(RFID_CHECK_RESPONSE_TOPIC)) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    String status = doc["status"];
    Serial.print("RFID Validacion: "); Serial.println(status);

    if (status == "VALID") {
      Serial.println("Acceso OK. Abriendo puerta principal.");
      servoPuertaPrincipal.write(90);
      puertaPrincipalState = "OPEN";
      publishShadowUpdate();
      delay(3000); 
      servoPuertaPrincipal.write(0);
      puertaPrincipalState = "CLOSED";
      publishShadowUpdate();
    } else {
      Serial.println("Acceso DENEGADO.");
      // Feedback visual opcional (blink led)
    }
    return;
  }

  // 2. ACTUALIZACIÓN (DELTA) O RECUPERACIÓN (GET/ACCEPTED)
  StaticJsonDocument<2048> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) { Serial.println("Error JSON"); return; }

  if (topicStr.equals(UPDATE_DELTA_TOPIC)) {
    if (doc.containsKey("state")) {
      aplicarCambios(doc["state"]);
    }
  } 
  else if (topicStr.equals(GET_SHADOW_ACCEPTED)) {
    Serial.println("Recuperando estado del Shadow...");
    if (doc.containsKey("state")) {
      JsonObject state = doc["state"];
      if (state.containsKey("desired")) {
        aplicarCambios(state["desired"]);
      } else if (state.containsKey("reported")) {
        aplicarCambios(state["reported"]);
      }
    }
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  pinMode(LED_HAB_PIN,  OUTPUT);
  pinMode(LED_COCINA_PIN, OUTPUT);

  servoPuertaPrincipal.attach(SERVO_PUERTA_PRINCIPAL_PIN);
  servoPuertaHab.attach(SERVO_PUERTA_HAB_PIN);
  servoPuertaCocina.attach(SERVO_PUERTA_COCINA_PIN);

  // Estado inicial físico seguro
  servoPuertaPrincipal.write(0);
  servoPuertaHab.write(0);
  servoPuertaCocina.write(0);
  digitalWrite(LED_HAB_PIN, LOW);
  digitalWrite(LED_COCINA_PIN, LOW);

  SPI.begin();
  mfrc522.PCD_Init();
  
  // Configuración de Certificados
  wiFiClient.setCACert(AMAZON_ROOT_CA1);
  wiFiClient.setCertificate(CERTIFICATE);
  wiFiClient.setPrivateKey(PRIVATE_KEY);

  // 1. Conexión WiFi con Gestor
  setupWiFi();

  // 2. Conexión MQTT
  mqttClient.setServer(MQTT_BROKER, 8883);
  mqttClient.setCallback(callback);
}

// --- LOOP ---
void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Lógica RFID
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String card_uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      char hex[3];
      sprintf(hex, "%02X", mfrc522.uid.uidByte[i]);
      card_uid += hex;
    }
    Serial.print("UID Detectado: "); Serial.println(card_uid);

    StaticJsonDocument<128> doc;
    doc["card_uid"] = card_uid;
    // Añadimos el nombre del Thing para que la Lambda sepa a quien responder
    doc["thing_name"] = THING_NAME; 
    
    char json_buffer[128];
    serializeJson(doc, json_buffer);

    mqttClient.publish(RFID_CHECK_REQUEST_TOPIC, json_buffer);
    Serial.println("Enviando a validar...");

    mfrc522.PICC_HaltA();
    delay(2000); 
  }
}