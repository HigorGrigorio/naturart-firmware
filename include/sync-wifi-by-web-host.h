/**
 * @file sync-wifi-by-web-host.h
 * @brief Syncs the WiFi credentials by web host.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 *
 */

#ifndef _SyncWiFiByWebHost_h
#define _SyncWiFiByWebHost_h

#include <wifi-connection.h>
#include <wifi-credentials.h>
#include <dns-server-factory.h>
#include <web-server-factory.h>
#include <blink-led.h>

/**
 * @brief Sync the WiFi credentials by local host
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto GetWiFiCredentilasFromUser() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing WiFi by local host...";

    // prepare the WiFi instance to Access Point.
    ConfigureWiFiToWebServer();

    DNSServer dnsServer;

    // configure dns server.
    ConfigureDNSServer(&dnsServer);

    // Need initialization of server.
    auto server = MakeWebServerBase();

    // contruct the handlers of web server.
    ConstructWebServerToWifiConfig(server);

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

#endif // ! _SyncWiFiByWebHost_h