/**
 * @file wifi-credentials.h
 * @brief WiFi credentials
 * @details This file contains declarations and functions for manipuling the credentials of the WiFi network.
 * @version 1.0
 * @date 2021-04-15
 *
 */

#ifndef _WiFiCredentials_h_
#define _WiFiCredentials_h_

#include <file.h>
#include <wifi-connection.h>
#include <global.h>

/**
 * @brief Get the WiFi credentials
 *
 * @return ErrorOr<WiFiCredentials> can be the credentials or failure()
 */
auto GetWiFiCredentials() -> ErrorOr<WiFiCredentials>
{
    auto readResult = ReadFromFile(SESSION_FILE, '\n');

    if (!readResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to read the session file.";
        return failure(readResult.error());
    }

    // unwrap the lines of file.
    auto lines = readResult.unwrap();

    // check if the file has 2 lines.
    if (lines.length() != 2)
    {
        INTERNAL_DEBUG() << "The session file has not 2 lines.";
        return failure({
            .context = "GetWiFiCredentials",
            .message = "The session file has not 2 lines",
        });
    }

    auto first = *lines.at(0);
    auto second = *lines.at(1);

    // remove the last character of the lines.
    return ok<WiFiCredentials>({
        .ssid = first.substring(0, first.length() - 1),
        .password = second.substring(0, second.length() - 1),
    });
}

#endif // ! _WiFiCredentials_h_