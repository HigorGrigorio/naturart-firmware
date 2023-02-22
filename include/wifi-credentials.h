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
#include <config/file-system.h>

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

/**
 * @brief Save the WiFi credentials
 *
 * @param credentials The credentials of the WiFi network
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SaveWiFiCredentials(WiFiCredentials credentials) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Saving WiFi credentials...";
    File file;

    auto createResult = CreateFile(SESSION_FILE);

    if (!createResult.ok())
    {
        INTERNAL_DEBUG() << createResult.error();
        auto openResult = OpenFile(SESSION_FILE, "w");

        if (!openResult.ok())
        {
            INTERNAL_DEBUG() << openResult.error();
            return failure({
                .context = "SaveWiFiCredentials",
                .message = "Failed to create and open the file",
            });
        }

        INTERNAL_DEBUG() << "File opened.";
        file = openResult.unwrap();
    }

    // Can't use else because of the return in the if statement.
    // Cannot open the file.
    if (!file)
    {
        return failure({
            .context = "SaveWiFiCredentials",
            .message = "Failed to open the file",
        });
    }

    file.println(credentials.ssid);
    file.println(credentials.password);

    CloseFile(file);

    return ok();
}

#endif // ! _WiFiCredentials_h_