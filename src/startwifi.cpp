#include "secrets.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include "startwifi.h"

//Credenziali del Wifi
//Invece che essere in chiaro nel codice
//sono memorizzate in un file separato
const char *ssid = SECRET_MONTIHOUSE_SSID;
const char *password = SECRET_MONTIHOUSE_PWD;

String LocalIP="";            // Indirizzo IP acquisito
String NetworkName = "Esp32"; // Nome con cui sara' visto in rete

// Setup estrapolato da setup del web server
void wifi_Setup(void){

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (MDNS.begin(NetworkName.c_str())) {
    Serial.println("MDNS responder started");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Name: ");
  Serial.println(NetworkName);

  LocalIP = WiFi.localIP().toString();
}

// Essendo estrapolato da un altro esempio, questo loop e' vuoto
void wifi_Loop(void){}