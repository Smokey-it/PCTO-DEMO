#ifndef WEBSERVERIMPL
#define WEBSERVERIMPL
#include <WebServer.h>
#define WEBSERVERPORT 80

//Variabili e tipi che saranno utilizzate dal main o altri moduli

//Porta di ascolto del server web. Default 80
extern int webserverport;
//Oggetto Web Server
extern  WebServer serverHTTP;

// Funzioni che saranno utilizzate dal main o altri moduli
void WebServer_Setup(void);
void WebServer_Loop(void);

#endif