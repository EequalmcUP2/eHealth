#include "ssid.h"

void wifi_connect(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting to WiFi...");
        delay(1000);
    }
    Serial.println("Connected to the WiFi network");
}