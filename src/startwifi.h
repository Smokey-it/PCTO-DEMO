/*
    Codice scorporato dagli esempi di web server
    Per trovare questo ed altri esempi:
    https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html
*/
#ifndef STARTWIFI
#define STARTWIFI

//Variabili e tipi che saranno utilizzate dal main o altri moduli

extern String LocalIP;      // Indirizzo IP acquisito
extern String NetworkName;  //Nome esposto in rete

// Funzioni che saranno utilizzate dal main o altri moduli
void wifi_Setup(void);
void wifi_Loop(void);

#endif