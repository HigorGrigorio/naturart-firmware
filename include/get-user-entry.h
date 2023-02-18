/**
 * @file get-sensor-credentials-from-user.h
 * @brief Gets the sensor credentials from the user.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2021-07-10
 *
 */

#ifndef _GetSensorCredentialsFromUser_h
#define _GetSensorCredentialsFromUser_h

#include <wifi-connection.h>
#include <sensor-credentials.h>
#include <web-server-factory.h>
#include <dns-server-factory.h>
#include <blink-led.h>

/**
 * @brief Sync the sensor credentials
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto GetUserEntryFromWebServer() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing sensor credentials from Naturart server...";

    // Requires WiFi to be disconnected to avoid conflicts with the web server.
    if (WiFi.isConnected())
    {
        WiFi.disconnect();
    }

    // prepare the WiFi instance to Access Point.
    ConfigureWiFiToWebServer();

    DNSServer dnsServer;

    // configure dns server.
    ConfigureDNSServer(&dnsServer);

    // Need initialization of server.
    auto server = MakeWebServerBase();

    // contruct the handlers of web server.
    ContructWebServerToUserCredentialsConfig(server);

    server.begin();
    TurnOnBuiltInLed();

    INTERNAL_DEBUG() << "Server started. Waiting for WiFi credentials...";

    while (true)
    {
        dnsServer.processNextRequest();
        delay(10);
    }

    /**
     * The ESP8266 will be restarted by the web server and the wifi credentials is saved in the file system.
     * Then the credentials will be read from the file system. So, this function will never return.
     */
    return ok();
}

#endif // ! _GetSensorCredentialsFromUser_h