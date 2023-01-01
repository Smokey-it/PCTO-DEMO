/*
    Esempio standard di utilizzo sensore temperatura 
    a cui ho aggiunto la memorizzazione delle ultime letture
    in un buffer circolare e delle funzioni per la presentazione
    dei dati rilevati su una pagina web
*/
#ifndef DHTIMPL
#define DHTIMPL

#include <CircularBuffer.h>
#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp
#include <Ticker.h>

#define SIZE_BUFFER 50 // Numero di letture da memorizzare

//Variabili e tipi che saranno utilizzate dal main o altri moduli

//Record per memorizzare entrambe le letture
struct DhtRecord{
  float Temp;
  float Humidity;
  DhtRecord(float T, float H):Temp(T),Humidity(H){}; // In C++ un record puo' avere costruttori
};

//Buffer circolare
extern CircularBuffer<DhtRecord*, SIZE_BUFFER> buffer;
//Oggetto per comandare il sensore
extern DHTesp dht;
//Attiva output su seriale
extern bool dht_serialDebug; 

// Funzioni che saranno utilizzate dal main o altri moduli

String svgTemperatureGraph(int=400,int=150);    // Genera grafico delle temperature in formato SVG
String htmlTemperatureTable(void);              // Genera tabella delle temperature in codice HTML
String htmlTemperatureHumidityTable(void);      // Genera tabella delle temperature e dell'umidita' in codice HTML
bool getTemperature(void);                      // Esegue una lettura e memorizza il dato nella coda circolare

void dht_Setup(void);
void dht_Loop(void);
#endif