// =================================================================
// ||  CÓDIGO ESP32 #2: ControlSala (WiFiManager + Shadow Sync)   ||
// =================================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Stepper.h>
#include <WiFiManager.h> // <-- REQUISITO: Librería para gestión de WiFi

// --- CONFIGURACIÓN DEL HARDWARE ---
#define PIR_PIN       23
#define LED_SALA_PIN  12 

// Motor paso a paso 28BYJ-48 + ULN2003
#define STEPS_PER_REV 2048
#define IN1 17
#define IN2 16
#define IN3 27
#define IN4 14

// --- CONFIGURACIÓN AWS ---
// NOTA: Ya no hay WIFI_SSID ni PASS aquí. Se gestionan por WiFiManager.
const char* MQTT_BROKER = "a30eisqpnafvzg-ats.iot.us-east-1.amazonaws.com";
const char* THING_NAME  = "ControlSala";

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
MIIDWTCCAkGgAwIBAgIUTsxmx0MP1qaGAjC1OXGlQ2mRRJAwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTIxMzAzNDgz
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKu7y0BArByJ40P639JC
OfJRFGKG52FIBSgvfrVx1w5e7aH/Hy4xeuieA/3QBD2OzYCdP14z6yJWubhbKtIP
Mt35co20CKUhuPg0j9T9he9AZh3AjPncBjrnIc5mbKn6VDIniK6flBds8gMIxMas
ElVa1jZJibadarFHPCORW9va9oW8xbM7JTgLtFDT3BT3iV0atGvatcp/8IfeWIkF
PqbqvEepcKtJ6JRzJrf0G1o+XlyBOX87c2TN577zI7G0+ssLdMr/AThQLJiGNSHy
mFsGAD8kWDjZOSRJvtjWzHAavfFDtNUsEfc0iskWlE09SRyVUUK4fUGVHpNIh4Cx
JV8CAwEAAaNgMF4wHwYDVR0jBBgwFoAU/IPcoibl0wdWZiXVE4MRxASYTe0wHQYD
VR0OBBYEFEiX4ZyjTZ7IgeHf2bzyjsgCveGYMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBCvyaz8lSPL7nZK4C1Jw2+4yxI
jwnapy6kLYZlumGP8nd9zKyqRe9uE5KTSZKUEEFuF8Xh5fUY5CjmPLRpwCAn74lG
ZAEIv9hWxkyDhwHgVVqJSSe1ETlTeqSZTRi7twKqeoGz8DPRBtfLQwOo1b9pxWZT
EsQhNCCEwKADapEtpxao2MTBC+N9ifFfaoYAfVPm/5bWW5iXNsm5VmJw1/2r3oiS
HT4G0snPiQOroz/3zKIVbu664/x9CR1+GKJlzZIqZsWPXvjd73V8eP7e3KGI0ZMT
KFrLWO8wXVUsNfmShqiJFS9C0RqOWAQxn8zy4a6F+p7AI2aczRZfrQ0Sqbcu
-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAq7vLQECsHInjQ/rf0kI58lEUYobnYUgFKC9+tXHXDl7tof8f
LjF66J4D/dAEPY7NgJ0/XjPrIla5uFsq0g8y3flyjbQIpSG4+DSP1P2F70BmHcCM
+dwGOuchzmZsqfpUMieIrp+UF2zyAwjExqwSVVrWNkmJtp1qsUc8I5Fb29r2hbzF
szslOAu0UNPcFPeJXRq0a9q1yn/wh95YiQU+puq8R6lwq0nolHMmt/QbWj5eXIE5
fztzZM3nvvMjsbT6ywt0yv8BOFAsmIY1IfKYWwYAPyRYONk5JEm+2NbMcBq98UO0
1SwR9zSKyRaUTT1JHJVRQrh9QZUek0iHgLElXwIDAQABAoIBAEAA4HJU6BBGz52h
XVN2fTQzRZ6m2osmFU3xzY6AG/9uH7B1bcCAjmctpR0uLrZmh258rIZGYUbN/a+Y
wq/BHFsgQbJQO7yXgeF3bXea+RS+8o/6GFBy0RuY0r+i6rK66jA7DbTPIplMcDHz
TashrD+FeDqJU7rsljv16ZLz9MFOEHUsvd8ye7fOpKfH/yvG3v/dmZOmacYahpnh
Npb/TLvlYTiWI3kI0EWqCaDA4izfFpFGtI/HV9svccdvWX62eheVR1SJ2l85hlJa
Yy0ULW58RBtyteUE+LBC/aeTOUT1vxAyoI0m+s+zURlkcyCbDDcNUKx0tnhzAbsa
q/7lVVkCgYEA3Zg2T/mzIMgjIRwMYSiOk4zOipJQKBKorErKcOt/0pWEw634qJUm
3K3xqoW+6gUxZ0wkE52wZ8Tk42KrWPweWWxed9mIax2/m7zVrnEZM4tYsJRKSZFe
RQLP6SznJlWRzg7rjYDYiU80ikyXYMd9vPv12UKv92yQSe3O4JcRj50CgYEAxmW9
ryFSUt/XrLkd5aTgVNZ27puxTDIRYxyOlKfN5BsEzMeFPhynMvKmbYZa2hcljz7C
u24lyqr+/Ff+CY2aepw3dvyU62MrCmB82VRBedRNZIx1Z2vSzxlFyVxlh+/t4UN/
H/apdTz5/OT0taJBQJ7st5H3s5Hcl4pl/QzuPisCgYBj1JAbZZ36lc+lufIlz1S0
Soo+SX5NH2LYA/XB+4ahg3TFh1nv8QoJ/19ReGGzIlXIeLgEElWH2l9XxXr2Ytb1
aGoCyoC36TYKXn9R0GgPHab+HkB9dj0nEAZEwVL9pS1DzNr36+Uhrwo3iBvmoNkX
y9dNYsrNyB4VAErkSYrCzQKBgAQHfvG2mIhsWWH9pHjMf23ZqTKxYMluXuIHa3Hj
cpAGaHOMPc2EIXUsbmrdBq1VOQMOwD+0Pfo+vMz9MICbKp8A2zOEb7XUHsSoLwDu
8CF8SVIYdC30h6SDG+K6CKnZGpivmT5Z8RkbcuxK7hleQTsBv0b/JOu1s6g0PFwG
TjRfAoGARhhhq3OWOhOrokZrA4kXstMkly5yuaJTEQKp3QG5lz2H/CmEWXAK29rS
PP0X7kH8cJRgt4Q8O6lho2/gg1kgEkvY0Zt9KLjEG9isMFt9E6dakZqsNBOLJpkt
sd4gqj/L3RfAJ1H1Lg4jXsEOTG7YkJuknw0iC9B+oykff7uxHYk=
-----END RSA PRIVATE KEY-----
)KEY";

// --- TEMAS MQTT ---
const char* UPDATE_TOPIC        = "$aws/things/ControlSala/shadow/update";
const char* UPDATE_DELTA_TOPIC  = "$aws/things/ControlSala/shadow/update/delta";

// REQUISITO 2: Temas para recuperar el estado (Persistence)
const char* GET_SHADOW_TOPIC    = "$aws/things/ControlSala/shadow/get";
const char* GET_SHADOW_ACCEPTED = "$aws/things/ControlSala/shadow/get/accepted";

// --- OBJETOS GLOBALES ---
WiFiClientSecure wiFiClient;
PubSubClient mqttClient(wiFiClient);
Stepper stepperMotor(STEPS_PER_REV, IN1, IN3, IN2, IN4);
WiFiManager wm; // Instancia de WiFiManager

// --- ESTADOS LÓGICOS ---
String luzSalaState        = "OFF";
String ventanaSalaState    = "CLOSED";
String movimientoSalaState = "NOT_DETECTED";

bool ventanaSalaAbierta = false;
volatile bool motionDetectedFlag = false;

// --- PROTOTIPOS ---
void publishShadowUpdate();
void reconnectMQTT();
void setupWiFi();
void abrirVentanaSala();
void cerrarVentanaSala();

// --- ISR PARA EL SENSOR PIR ---
void IRAM_ATTR pirISR() {
  motionDetectedFlag = true;
}

// --- CONFIGURACIÓN WIFI CON WIFIMANAGER ---
void setupWiFi() {
  Serial.println("\nIniciando WiFiManager...");
  // Si quieres resetear las credenciales para probar, descomenta la siguiente línea:
  // wm.resetSettings();
  
  // Crea un Access Point si no puede conectarse
  // Nombre del AP: "ControlSala_AP", Sin contraseña (o añade una)
  bool res = wm.autoConnect("ControlSala_AP"); 

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
    Serial.print("Conectando MQTT como 'ControlSala'...");
    if (mqttClient.connect(THING_NAME)) {
      Serial.println("Conectado a AWS IoT.");
      
      // Suscripciones
      mqttClient.subscribe(UPDATE_DELTA_TOPIC); // Para recibir órdenes en tiempo real
      mqttClient.subscribe(GET_SHADOW_ACCEPTED); // Para recibir el estado guardado al iniciar
      
      Serial.println("Suscrito a delta y get/accepted.");
      
      // REQUISITO 2: Solicitar el último estado conocido a AWS (Recuperación)
      // Publicamos un mensaje vacío al tema /get
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

  reported["luz_sala"]       = luzSalaState;
  reported["ventana_sala"]   = ventanaSalaState;
  reported["movimiento_sala"]= movimientoSalaState;

  char buffer[512];
  serializeJson(doc, buffer);
  mqttClient.publish(UPDATE_TOPIC, buffer);
  Serial.println("Shadow reportado:");
  Serial.println(buffer);
}

// --- FUNCIONES DE ACTUADORES ---
void abrirVentanaSala() {
  if (!ventanaSalaAbierta) {
    Serial.println("Abriendo ventana...");
    stepperMotor.setSpeed(12);
    stepperMotor.step(STEPS_PER_REV / 4);
    ventanaSalaAbierta = true;
    Serial.println("Ventana abierta.");
  }
}

void cerrarVentanaSala() {
  if (ventanaSalaAbierta) {
    Serial.println("Cerrando ventana...");
    stepperMotor.setSpeed(12);
    stepperMotor.step(-STEPS_PER_REV / 4);
    ventanaSalaAbierta = false;
    Serial.println("Ventana cerrada.");
  }
}

// --- LÓGICA DE APLICACIÓN DE ESTADO ---
// Esta función evita duplicar código en el callback
void aplicarCambios(JsonObject state) {
  bool changed = false;

  // --- LUZ SALA ---
  if (state.containsKey("luz_sala")) {
    String desired = state["luz_sala"];
    if (desired != luzSalaState) {
      Serial.print("Aplicando Luz Sala: "); Serial.println(desired);
      digitalWrite(LED_SALA_PIN, desired == "ON" ? HIGH : LOW);
      luzSalaState = desired;
      changed = true;
    }
  }

  // --- VENTANA SALA ---
  if (state.containsKey("ventana_sala")) {
    String desired = state["ventana_sala"];
    if (desired != ventanaSalaState) {
      Serial.print("Aplicando Ventana Sala: "); Serial.println(desired);
      if (desired == "OPEN") {
        abrirVentanaSala();
      } else {
        cerrarVentanaSala();
      }
      ventanaSalaState = desired;
      changed = true;
    }
  }

  if (changed) {
    Serial.println("Estado sincronizado, reportando a AWS...");
    publishShadowUpdate();
  }
}

// --- CALLBACK MQTT ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n--- INICIO CALLBACK ---");
  Serial.print("Mensaje recibido en: ");
  Serial.println(topic);
  
  StaticJsonDocument<2048> doc; // Aumentamos tamaño para recibir el Shadow completo
  DeserializationError error = deserializeJson(doc, payload, length);
  
  if (error) {
    Serial.print("Error JSON: ");
    Serial.println(error.c_str());
    return;
  }

  String topicStr = String(topic);

  // CASO 1: Recibimos una orden de cambio (DELTA)
  if (topicStr.equals(UPDATE_DELTA_TOPIC)) {
    if (doc.containsKey("state")) {
      aplicarCambios(doc["state"]);
    }
  }
  
  // CASO 2: Recibimos el estado completo al conectar (RECOVERY / PERSISTENCE)
  else if (topicStr.equals(GET_SHADOW_ACCEPTED)) {
    Serial.println("Recibido estado completo del Shadow (Recuperación).");
    
    // El Shadow completo tiene "state" -> "desired" y "state" -> "reported"
    // Damos prioridad a "desired" si existe, si no, usamos "reported"
    if (doc.containsKey("state")) {
      JsonObject state = doc["state"];
      if (state.containsKey("desired")) {
        Serial.println("Sincronizando con estado DESEADO...");
        aplicarCambios(state["desired"]);
      } else if (state.containsKey("reported")) {
        Serial.println("Sincronizando con estado REPORTADO...");
        aplicarCambios(state["reported"]);
      }
    }
  }
  
  Serial.println("--- FIN CALLBACK ---\n");
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  pinMode(LED_SALA_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT_PULLUP);
  
  stepperMotor.setSpeed(10);
  
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirISR, CHANGE);

  // Configuración segura AWS
  wiFiClient.setCACert(AMAZON_ROOT_CA1);
  wiFiClient.setCertificate(CERTIFICATE);
  wiFiClient.setPrivateKey(PRIVATE_KEY);

  // 1. Conexión WiFi (Gestionada por WiFiManager)
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

  if (motionDetectedFlag) {
    motionDetectedFlag = false;
    delay(50); // Debounce
    
    String nuevoEstado = digitalRead(PIR_PIN) == HIGH ? "DETECTED" : "NOT_DETECTED";
    
    if (nuevoEstado != movimientoSalaState) {
      movimientoSalaState = nuevoEstado;
      Serial.print("Cambio sensor movimiento: ");
      Serial.println(movimientoSalaState);
      publishShadowUpdate();
    }
  }
}