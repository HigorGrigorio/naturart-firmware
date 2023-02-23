/**
 * @file get-sensor-credentials-from-broker.h
 * @brief Gets the sensor credentials from the broker.
 * @details This file contains the functions to get the sensor credentials from the broker.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 *
 */

#ifndef _GetSensorCredentialsFromBroker_h
#define _GetSensorCredentialsFromBroker_h

#include <PubSubClient.h>

#include <sensor-typing.h>
#include <user-entry.h>
#include <wifi-connection.h>
#include <uuid-factory.h>

/**
 * @brief Sync the sensor credentials by local host.
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto GetSensorCredentialsFromBroker(String &id) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing sensor credentials by naturart broker...";

    if (WiFi.status() != WL_CONNECTED)
    {
        INTERNAL_DEBUG() << "WiFi is not connected";

        CleanFile(SESSION_FILE);
        ESP.restart();
    }

    // using a fake broker to monitorize the connection.
    const char *host = "broker.hivemq.com";

    String uuid = makeUUID();

    WiFiClient espClient;

    PubSubClient client(espClient);

    client.setServer(host, 1883);

    INTERNAL_DEBUG() << "Connecting to MQTT broker...";

    auto reconnect = [&client]() -> void
    {
        while (!client.connected())
        {

            // Create a random client ID
            String clientId = "ESP8266Client-";
            clientId += String(random(0xffff), HEX);

            // Attempt to connect
            if (client.connect(clientId.c_str()))
            {
                INTERNAL_DEBUG() << "Connected to MQTT broker";
            }
        }
    };

    reconnect();

    client.setCallback([&](char *topic, byte *payload, unsigned int length) -> void
                       {
                        if(uuid != topic) {
                            INTERNAL_DEBUG() << "Invalid topic. Ignoring...";
                            return; 
                        }

                        String spayload = "";
                        spayload.reserve(length);

                        for (unsigned int i = 0; i < length; i++)
                        {
                            spayload += (char)payload[i];
                        }

                        INTERNAL_DEBUG() << "Message arrived [" << topic << "]: " << spayload;


                        auto parseResult = CredentialsFromBrokerPayload(spayload);

                        if (!parseResult.ok())
                        {
                            INTERNAL_DEBUG() << "Could not extract credentials from json";
                            INTERNAL_DEBUG() << parseResult.error();
                            // forces esp to collect the data again from the user
                            CleanFile(ENTRY_FILE);
                        }
                        else
                        {
                            auto credentials = parseResult.unwrap();

                            if (credentials.length() == 0)
                            {
                                INTERNAL_DEBUG() << "Invalid credentials number";

                                // forces esp to collect the data again from the user
                                CleanFile(ENTRY_FILE);
                            }
                            else
                            {
                                auto saveResult = SaveSensorCredentials(credentials);

                                if (!saveResult.ok())
                                {
                                    INTERNAL_DEBUG() << saveResult.error();
                                }
                                else
                                {
                                    INTERNAL_DEBUG() << "Saved sensor credentials";
                                }
                            }
                        }

                        ESP.restart(); });

    client.subscribe(uuid.c_str());

    INTERNAL_DEBUG() << "Subcribed on topic '" << uuid.c_str() << "'";

    String json = "{\"uuid\": \"" + uuid + "\", \"id\": \"" + id + "\"}";

    client.publish("credentials", json.c_str());

    // Await the payload of broker to restart.
    while (true)
    {
        if (!client.connected())
        {
            reconnect();
        }

        client.loop();
        delay(10);
    }

    // Never return.
    return ok();
}

#endif // ! _GetSensorCredentialsFromBroker_h