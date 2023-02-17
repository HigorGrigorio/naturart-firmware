/**
 * @file get-sensor-credentials-from-broker.h
 * @brief Gets the sensor credentials from the broker.
 * @details This file contains the functions to get the sensor credentials from the broker.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2021-07-10
 * 
*/

#ifndef _GetSensorCredentialsFromBroker_h
#define _GetSensorCredentialsFromBroker_h

#include <PubSubClient.h>

#include <sensor-credentials.h>
#include <user-entry.h>
#include <wifi-connection.h>

/**
 * @brief Sync the sensor credentials by local host.
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto GetSensorCredentialsFromBroker(UserEntry &entry) -> ErrorOr<SensorCredentials>
{
    INTERNAL_DEBUG() << "Syncing sensor credentials by naturart broker...";
    if (WiFi.status() != WL_CONNECTED)
    {
        INTERNAL_DEBUG() << "WiFi is not connected";
        ESP.restart();
        return failure({
            .context = "TryGetSensorCredentials",
            .message = "WiFi is not connected",
        });
    }

    // using a fake broker to monitorize the connection.
    const char *host = "broker.hivemq.com";
    bool receive = true;

    // TODO: get the real uuid.
    entry.id = "123456789";

    // TODO: impl a real feature. This is just a test.
    //   The real feature just need to send the user entry to server and subscribe to the topic with the id
    // of the user entry. Then the server will send the result of entry in the topic. On receiving the result,
    // the device will save the result to the file system.

    // The WiFi client
    WiFiClient wifiClient;

    // The MQTT client
    PubSubClient mqttClient(wifiClient);
    
    mqttClient.setServer(host, 1883);

    INTERNAL_DEBUG() << "Connecting to MQTT broker...";

    while (!mqttClient.connected())
    {
        mqttClient.connect("", "", "");
        delay(500);
    }

    INTERNAL_DEBUG() << "Connected to MQTT broker";

    mqttClient.subscribe("sync");
    mqttClient.setCallback([&](char *topic, byte *payload, unsigned int length) -> void
                           { INTERNAL_DEBUG() << "Message arrived [" << topic << "] " << (char *)payload; receive = true; });

    mqttClient.publish("sync", entry.ToJson().c_str());

    while (!receive)
    {
        delay(10);
    }

    INTERNAL_DEBUG() << "Received";
    return ok<SensorCredentials>({});
}


#endif // ! _GetSensorCredentialsFromBroker_h