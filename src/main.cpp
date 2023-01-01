#include <Arduino.h>

#include "startwifi.h"
#include "dhtimpl.h"
#include "webserverImpl.h"

void (*reset)(void) = 0;

void setup()
{
  /*
  //Se decido di usare una funzione main devo chiamare queste funzioni
  init();
  initVariant();
  initArduino();
  */

  Serial.begin(115200);
  Serial.println();
  
  //Inizializza il sensore
  dht_Setup();

  // Posso impostare qui le funzioni del server web
  // sarebbe meglio farlo in una funzione
  serverHTTP.on("/debugDHT", []() {
    String messaggio = "Debug DHT attivato";
    dht_serialDebug = false;
    Serial.println(messaggio);
    serverHTTP.send(200, "text/plain", messaggio);
  });
  serverHTTP.on("/nodebugDHT", []() {
    String messaggio = "Debug DHT disattivato";
    dht_serialDebug = false;
    Serial.println(messaggio);
    serverHTTP.send(200, "text/plain", messaggio);
  });

  //Inizializza il wifi
  wifi_Setup();  
  
  //Inizializza il server web
  WebServer_Setup();  
}

void loop() {
  
  dht_Loop();
  wifi_Loop();
  WebServer_Loop();
}