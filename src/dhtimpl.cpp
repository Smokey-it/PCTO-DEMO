#include "dhtimpl.h"

CircularBuffer<DhtRecord *, SIZE_BUFFER> buffer;

DHTesp dht;
bool dht_serialDebug = false;

/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
/** Pin number for DHT11 data pin */
int dhtPin = 17;

// Modifica rispetto ad esempio originale
// Se l'alimentazione al sensore viene fornita tramite pin
// diventa possibile attivarlo e disattivarlo.
// Altrimenti impostarlo a zero
int powerPin = 16;

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
 */
bool getTemperature()
{
    // Reading temperature for humidity takes aboutHTML 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    TempAndHumidity newValues = dht.getTempAndHumidity();
    // Check if any reads failed and exit early (to try again).
    if (dht.getStatus() != 0)
    {
        Serial.println("DHT11 error status: " + String(dht.getStatusString()));
        return false;
    }

    float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
    float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
    float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

    String comfortStatus;
    switch (cf)
    {
    case Comfort_OK:
        comfortStatus = "Comfort_OK";
        break;
    case Comfort_TooHot:
        comfortStatus = "Comfort_TooHot";
        break;
    case Comfort_TooCold:
        comfortStatus = "Comfort_TooCold";
        break;
    case Comfort_TooDry:
        comfortStatus = "Comfort_TooDry";
        break;
    case Comfort_TooHumid:
        comfortStatus = "Comfort_TooHumid";
        break;
    case Comfort_HotAndHumid:
        comfortStatus = "Comfort_HotAndHumid";
        break;
    case Comfort_HotAndDry:
        comfortStatus = "Comfort_HotAndDry";
        break;
    case Comfort_ColdAndHumid:
        comfortStatus = "Comfort_ColdAndHumid";
        break;
    case Comfort_ColdAndDry:
        comfortStatus = "Comfort_ColdAndDry";
        break;
    default:
        comfortStatus = "Unknown:";
        break;
    };

    // Aggiunta memorizzazione dei valori in un buffer circolare
    buffer.push(new DhtRecord(newValues.temperature, newValues.humidity));
    // Creata condizione per pilotare output su seriale che non serve piu' se non per debug
    if (dht_serialDebug)
    {
        Serial.println(" T:" + String(newValues.temperature) + "\tH:" + String(newValues.humidity) + "\tI:" + String(heatIndex) + "\tD:" + String(dewPoint) + " " + comfortStatus);
    }
    return true;
}

// Genera codice SVG per il grafico delle temperature
String svgTemperatureGraph(int rectX, int rectY)
{
    String sRectX, sRectY;
    int x, y, y2;
    char tempStr[100];

    int nMeasures = buffer.size();
    int xStep = rectX / nMeasures;

    // Min Max Temperature in the list
    float minT = 9999, maxT = -9999;
    for (int i = 0; i < buffer.size(); i++)
    {
        minT = minT > buffer[i]->Temp ? buffer[i]->Temp : minT;
        maxT = maxT < buffer[i]->Temp ? buffer[i]->Temp : maxT;
    }
    // Allowance for temperature range
    float extraY = (maxT - minT) * 0.1;
    extraY += extraY < 1 ? 1 : 0;

    sprintf(tempStr, "%d", rectX);
    sRectX = tempStr;
    sprintf(tempStr, "%d", rectY);
    sRectY = tempStr;

    // Contenitore e sfondo
    String outHTML = "";
    outHTML += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" + sRectX + "\" height=\"" + sRectY + "\">\n";
    outHTML += "<rect width=\"" + sRectX + "\" height=\"" + sRectY + "\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
    outHTML += "<g stroke=\"black\">\n";

    // Bordo sx
    sprintf(tempStr, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", 0, 0, 0, rectY);
    outHTML += tempStr;
    // Linee di livello
    y = map(maxT, minT - extraY, maxT + extraY, 0, rectY);
    sprintf(tempStr, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", 0, rectY - y, 5, rectY - y);
    outHTML += tempStr;
    y = map(minT, minT - extraY, maxT + extraY, 0, rectY);
    sprintf(tempStr, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, rectY - y, 5, rectY - y);
    outHTML += tempStr;

    // Punti del grafico
    int i = 0;
    y = map(buffer[i++]->Temp, minT - extraY, maxT + extraY, 0, rectY);
    sprintf(tempStr, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" />\n", 0, rectY - y, 3);
    for (x = 0; x < rectX; x += xStep)
    {
        y2 = map(buffer[i++]->Temp, minT - extraY, maxT + extraY, 10, 130);
        sprintf(tempStr, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, rectY - y, x + xStep, rectY - y2);
        outHTML += tempStr;
        sprintf(tempStr, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\"  stroke=\"red\" />\n", x + xStep, rectY - y2, 2);
        outHTML += tempStr;
        y = y2;
    }

    outHTML += "</g>\n</svg>\n";

    return outHTML;
}

// Genera codice HTML per la tabella delle temperature
String htmlTemperatureTable()
{
    char tempStr[100];

    int nMeasures = buffer.size();

    // Contenitore
    String outHTML = "";
    outHTML += "<style>table, th, td {border: 1px solid black;}</style>";
    outHTML += "<table>\n";
    outHTML += "<tr>\n";
    outHTML += "<th>Ordine</th><th>Temp C</th>\n";
    outHTML += "</tr>\n";

    // Valori rilevati
    for (int i = 0; i < nMeasures; i++)
    {
        outHTML += "<tr>\n";
        sprintf(tempStr, "<td>%d</td>", i + 1);
        outHTML += tempStr;
        sprintf(tempStr, "<td>%0.2f</td>", buffer[i]->Temp);
        outHTML += tempStr;
        outHTML += "</tr>\n";
    }

    outHTML += "</table>\n";
    return outHTML;
}

// Genera codice HTML per la tabella delle temperature e umidita'
String htmlTemperatureHumidityTable()
{
    char tempStr[100];

    int nMeasures = buffer.size();

    // Contenitore
    String outHTML = "";
    outHTML += "<style> table, th, td {border: 1px solid black; border-collapse: collapse;} </style>";
    outHTML += "<style> td {  text-align: center;} </style>";
    outHTML += "<table>\n";
    outHTML += "<tr>\n";
    outHTML += "<th>Ordine</th><th>Temp C</th><th>Humidity %</th>\n";
    outHTML += "</tr>\n";

    // Valori rilevati
    for (int i = 0; i < nMeasures; i++)
    {
        outHTML += "<tr>\n";
        sprintf(tempStr, "<td>%d</td>", i + 1);
        outHTML += tempStr;
        sprintf(tempStr, "<td>%0.2f</td>", buffer[i]->Temp);
        outHTML += tempStr;
        sprintf(tempStr, "<td>%0.0f%</td>", buffer[i]->Humidity);
        outHTML += tempStr;
        outHTML += "</tr>\n";
    }

    outHTML += "</table>\n";
    return outHTML;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp()
{
    if (tempTaskHandle != NULL)
    {
        xTaskResumeFromISR(tempTaskHandle);
    }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters)
{
    Serial.println("tempTask loop started");
    while (1) // tempTask loop
    {
        if (tasksEnabled)
        {
            // Get temperature values
            getTemperature();
        }
        // Got sleep again
        vTaskSuspend(NULL);
    }
}

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
bool initTemp()
{
    byte resultValue = 0;
    // Initialize temperature sensor
    dht.setup(dhtPin, DHTesp::DHT11);
    Serial.println("DHT initiated");

    // Start task to get temperature
    xTaskCreatePinnedToCore(
        tempTask,        /* Function to implement the task */
        "tempTask ",     /* Name of the task */
        4000,            /* Stack size in words */
        NULL,            /* Task input parameter */
        5,               /* Priority of the task */
        &tempTaskHandle, /* Task handle. */
        1);              /* Core where the task should run */

    if (tempTaskHandle == NULL)
    {
        Serial.println("Failed to start task for temperature update");
        return false;
    }
    else
    {
        // Start update of environment data every 20 seconds
        tempTicker.attach(10, triggerGetTemp);
    }
    return true;
}

// Setup dell'esempio originale
void dht_Setup(void)
{
    Serial.println("DHT ESP32 example with tasks");

    // Modificato rispetto ad esempio standard per pilotare il sensore
    if (powerPin)
    {
        pinMode(powerPin, OUTPUT);
        digitalWrite(powerPin, HIGH);
    }

    initTemp();
    // Signal end of setup() to tasks
    tasksEnabled = true;
}

// Loop dell'esempio originale
void dht_Loop(void)
{
    if (!tasksEnabled)
    {
        // Wait 2 seconds to let system settle down
        delay(2000);
        // Enable task that will read values from the DHT sensor
        tasksEnabled = true;
        if (tempTaskHandle != NULL)
        {
            vTaskResume(tempTaskHandle);
        }
    }
    yield();
}