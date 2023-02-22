/**
 * @file main.cpp
 * @brief The main file of the project
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 */

#include <config/system.h>
#include <config/dns-server.h>
#include <config/file-sistem.h>
#include <config/web-server.h>

#include <Arduino.h>

#include <PubSubClient.h>

#include <sync-sensor-credentials.h>
#include <sync-wifi.h>

void setup()
{
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    if (!LittleFS.begin())
    {
        INTERNAL_DEBUG() << "Failed to mount file system";
        return;
    }

    auto result1 = SyncWiFi();

    if (!result1.ok())
    {
        INTERNAL_DEBUG() << result1.error();
        return;
    }

    auto result2 = SyncSensor();

    if (!result2.ok())
    {
        INTERNAL_DEBUG() << result2.error();
        return;
    }

    INTERNAL_DEBUG() << "Synced successfully";

    auto credentials = GetSensorCredentials();
}

void loop()
{
    delay(0);
}