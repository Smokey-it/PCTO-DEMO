/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
  Esempio originale con la modifica delle pagine HTML  
*/

#include <WebServer.h>
#include "webserverImpl.h"
#include "dhtImpl.h"

int webserverport = WEBSERVERPORT;
WebServer serverHTTP;

const int led = 13;

/*
  Genera il codice HTML di intestazione pagina
  dove e' possibile impostare un tempo di refresh
  automatico per il browser. Con zero non viene usato
*/
String HtmlHead(int refreshSeconds)
{
  String outHTML = "";
  char temp[400];

  outHTML += "<html>";
  outHTML += "<head>";
  if (refreshSeconds > 0)
  {
    snprintf(temp, 400, "<meta http-equiv='refresh' content='%d'/>", refreshSeconds);
    outHTML += temp;
  }
  outHTML += "<title>Monti MiniWebServer</title>";
  outHTML += "<style>";
  outHTML += "body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }";
  outHTML += "</style>";
  outHTML += "</head>";
  return outHTML;
}

/*
  Pagina Web iniziale
*/
void handleRoot()
{
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String outHTML = HtmlHead(10);
  outHTML += "<body>";
  outHTML += "<h1>Hello from Monti Mini Web Server!</h1>";
  snprintf(temp, 400, " <p>Uptime: %02d:%02d:%02d</p>", hr, min % 60, sec % 60);
  outHTML += temp;
  outHTML += "<img src=\"/temp.svg\" />";
  snprintf(temp, 400, "<p>Temperature measured over %d points</p>", buffer.size());
  outHTML += temp;
  outHTML += htmlTemperatureHumidityTable();
  outHTML += "</body>";
  outHTML += "</html>";
  serverHTTP.send(200, "text/html", outHTML);
  digitalWrite(led, 0);
}

/*
  Questa funzione viene richiamata quando il browser
  richiede una pagina che non esiste.
  Per testare la funzione basta usare un browser
  e cercare per esempio:
  http://192.168.1.138/pippo?argomento=niente
  (sostituire indirizzo IP del modulo)
*/
void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += serverHTTP.uri();
  message += "\nMethod: ";
  message += (serverHTTP.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += serverHTTP.args();
  message += "\n";

  for (uint8_t i = 0; i < serverHTTP.args(); i++)
  {
    message += " " + serverHTTP.argName(i) + ": " + serverHTTP.arg(i) + "\n";
  }

  serverHTTP.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

/*
  Genera il grafico delle temperature e lo restituisce al client
*/
void drawTemperatureGraph()
{
  String out = svgTemperatureGraph(400, 150);
  serverHTTP.send(200, "image/svg+xml", out);  
}

/*
  Imposta le pagine del server
*/
void webServer_Pages(void)
{
  serverHTTP.on("/", handleRoot);
  serverHTTP.on("/inline", []()
                { serverHTTP.send(200, "text/plain", "this works as well"); });
  serverHTTP.on("/temp.svg", drawTemperatureGraph);

  serverHTTP.onNotFound(handleNotFound);
}

void WebServer_Setup(void)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);  

  webServer_Pages();

  serverHTTP.begin(webserverport);
  Serial.println("HTTP server started");
}

// Loop dell'esempio originale
void WebServer_Loop(void)
{
  serverHTTP.handleClient();
  delay(2); // allow the cpu to switch to other tasks
}
