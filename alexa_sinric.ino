#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// **************************************************
//    fichiers de config créé
// **************************************************
#include "credentials.h"


ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;



uint64_t heartbeatTimestamp = 0;
bool isConnected = false;




// **************************************************
//    constantes ajoutées
// **************************************************
const String relais_1  = "5fa3c33ab1c8c45d66217d08";      // id crée par le site sinric.com
const int    pinRelais_1  = D2;                           // broche utilisé sur l'ESP

const String relais_2  = "5fa3c382b1c8c45d66217d0e";     // id crée par le site sinric.com
const int    pinRelais_2  = D3;                          // broche utilisé sur l'ESP

const String relais_3  = "5fa3c392b1c8c45d66217d10";     // id crée par le site sinric.com
const int    pinRelais_3  = D4;                          // broche utilisé sur l'ESP

const String relais_4  = "5fa3c3aab1c8c45d66217d14";     // id crée par le site sinric.com
const int    pinRelais_4  = D5;                          // broche utilisé sur l'ESP
// **************************************************


// **************************************************
//  fonctions turnOn  et turnOff supprimées => fonction turnOnOff crée
// **************************************************

void turnOnOff(String deviceId, int value) {
  if (deviceId == relais_1)  digitalWrite( pinRelais_1, value);   // broche D2 sortie HIGH ou LOW selon value
  if (deviceId == relais_2)  digitalWrite( pinRelais_2, value);   // broche D3 sortie HIGH ou LOW selon value
  if (deviceId == relais_3)  digitalWrite( pinRelais_3, value);   // broche D4 sortie HIGH ou LOW selon value
  if (deviceId == relais_4)  digitalWrite( pinRelais_4, value);   // broche D5 sortie HIGH ou LOW selon value
}
// **************************************************



void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      isConnected = false;
      Serial.printf("[WSc] Webservice déconnecté de sinric.com!\n");
      break;
    case WStype_CONNECTED: {
        isConnected = true;
        Serial.printf("[WSc] Service connecté à sinric.com à l'url: %s\n", payload);
        Serial.printf("Waiting for commands from sinric.com ...\n");
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);


#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);
#endif

        // Example payloads
        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        //       if(action == "setPowerState") { // Switch or Light
        //       if (action == "SetTargetTemperature")



        // **************************************************
        //         partie modifiée
        // **************************************************
        String deviceId = json ["deviceId"];
        String value = json ["value"];
        if (value == "OFF") {
          turnOnOff(deviceId, 1);
        } else {
          turnOnOff(deviceId, 0);
        }
      }
      break;

    default: break;
  }
}

void setup() {
  Serial.begin(115200);

  // ********************************************************************************
  //  Broches mises en sortie
  // *******************************************************************************
  pinMode(pinRelais_1, OUTPUT);              // broche D2 en sortie
  pinMode(pinRelais_2, OUTPUT);              // broche D3 en sortie
  pinMode(pinRelais_3, OUTPUT);              // broche D4 en sortie
  pinMode(pinRelais_4, OUTPUT);              // broche D5 en sortie
  //***********************************************************************************

  


  // ********************************************************************************
  //  Broches à HIGH
  // *******************************************************************************
  digitalWrite( pinRelais_1, HIGH);          // broche D2 à l'état HIGH
  digitalWrite( pinRelais_2, HIGH);          // broche D3 à l'état HIGH
  digitalWrite( pinRelais_3, HIGH);          // broche D4 à l'état HIGH
  digitalWrite( pinRelais_4, HIGH);          // broche D5 à l'état HIGH
  //***********************************************************************************

  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);


  // En attente de connexion Wifi
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // adresse, port et URL du serveur
  webSocket.begin("iot.sinric.com", 80, "/");

  // gestionnaire d'événements
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);

  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  webSocket.loop();
  if (isConnected) {
    uint64_t now = millis();

    // Envoie une impulsion afin d'éviter les déconnexions lors de la réinitialisation des adresses IP par le FAI pendant la nuit. Merci @MacSass

    if ((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
      heartbeatTimestamp = now;
      webSocket.sendTXT("H");
    }
  }
}
