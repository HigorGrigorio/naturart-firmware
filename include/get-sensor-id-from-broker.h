/**
 * @file get-sensor-id-from-broker.h
 * @brief Gets the sensor id from the broker.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0
 * @date 2019-04-10
 *
 */

#ifndef _GetSensorIdFromBroker_h_
#define _GetSensorIdFromBroker_h_

#include <user-entry.h>
#include <wifi-connection.h>
#include <uuid-factory.h>
#include <sensor-self.h>

#include <PubSubClient.h>

/**
 * @brief Gets the sensor id from the broker.
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto GetSensorIdFromBroker(UserEntry &entry) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing sensor id by naturart broker...";

    if (WiFi.status() != WL_CONNECTED)
    {
        INTERNAL_DEBUG() << "WiFi is not connected";

        CleanFile(SESSION_FILE);
        ESP.restart();
    }

    // using a fake broker to monitorize the connection.
    const char *host = "broker.hivemq.com";

    entry.id = makeUUID();

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
                        if(entry.id != topic) {
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


                        auto parseResult = SelfFromBrokerPayload(spayload);

                        if (!parseResult.ok())
                        {
                            INTERNAL_DEBUG() << "Could not extract credentials from json";
                            INTERNAL_DEBUG() << parseResult.error();
                            // forces esp to collect the data again from the user
                            CleanFile(ENTRY_FILE);
                        }
                        else
                        {
                            auto id = parseResult.unwrap();

                            if (id.length() == 0)
                            {
                                INTERNAL_DEBUG() << "Invalid credentials number";

                                // forces esp to collect the data again from the user
                                CleanFile(ENTRY_FILE);
                            }
                            else
                            {
                                auto saveResult = SaveSelf(id);

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

    client.subscribe(entry.id.c_str());

    INTERNAL_DEBUG() << "Subcribed on topic '" << entry.id.c_str() << "'";

    client.publish("sync", entry.ToJson().c_str());

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

#endif // ! _GetSensorIdFromBroker_h_