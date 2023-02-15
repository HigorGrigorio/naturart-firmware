/**
 * @file wifi-connection.h
 * @brief defines the function ConnectWifi
 * @details This file contains the function ConnectWifi which is used to connect to the wifi network
 * @version 1.0
 * @date 2021-04-15
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 *
 */

#ifndef _ConnectWifi_h_
#define _ConnectWifi_h_

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <ErrorOr.h>

/**
 * @brief The credentials of a WiFi network
 */
struct WiFiCredentials
{
    String ssid = "";
    String password = "";
};

/**
 * @brief Connect to a WiFi network
 *
 * @param WiFiCredentials The credentials of the WiFi network
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto WiFiConnect(WiFiCredentials &credentials) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Connecting to WiFi...";

    WiFi.begin(credentials.ssid, credentials.password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        return failure({
            .context = "WiFiConnect",
            .message = "Failed to connect to WiFi",
        });
    }

    return ok();
}

/**
 * @brief Disconnect from a WiFi network
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto WiFiDisconnect() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Disconnecting from WiFi...";
    WiFi.disconnect(true);
    return ok();
}

#endif // ! _ConnectWifi_h_
