/**
 * @file main.cpp
 *
 * @brief The main file of the project
 *
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 */

#include <global.h>

#include <Arduino.h>
#include <LittleFS.h>

#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

#include <ErrorOr.h>
#include <Guard.h>
#include <Nonnull.h>
#include <Check.h>
#include <UtilStringArray.h>

/**
 * @brief Write in a file
 *
 * @param path The path of the file
 * @param content The content to write
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto WriteInFile(String path, String content, bool endline) -> ErrorOr<>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "WriteInFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        return failure({
            .context = "WriteInFile",
            .message = "Failed to open the file",
        });
    }

    if (endline ? file.println(content) : file.print(content))
    {
        return ok();
    }

    return failure({
        .context = "WriteInFile",
        .message = "Failed to write in the file",
    });
}

/**
 * @brief Read a file
 *
 * @param path The path of the file
 * @param end The end of the line
 *
 * @return ErrorOr<utility::StringArray> can be the lines of the file or failure().
 */
auto ReadFromFile(String path, char end) -> ErrorOr<utility::StringArray>
{
    INTERNAL_DEBUG() << "ReadFromFile: " << path;

    if (!LittleFS.exists(path))
        return failure({
            .context = "ReadFromFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        return failure({
            .context = "ReadFromFile",
            .message = "Failed to open the file",
        });
    }

    utility::StringArray lines;
    String buff = "";

    while (file.available() > 0)
    {
        char c = (char)file.read();

        if (c == end)
        {
            lines.add(buff);
            buff = "";
            continue;
        }
        buff += c;
    }

    INTERNAL_DEBUG() << "ReadFromFile: " << lines.length() << " lines";

    return ok(lines);
}

/**
 * @brief Delete a file
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto DeleteFile(String path) -> ErrorOr<>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "DeleteFile",
            .message = "File does not exist",
        });

    if (LittleFS.remove(path))
        return ok();

    return failure({
        .context = "DeleteFile",
        .message = "Failed to delete the file",
    });
}

/**
 * @brief Clean a file
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto CleanFile(String path) -> ErrorOr<>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "CleanFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        return failure({
            .context = "CleanFile",
            .message = "Failed to open the file",
        });
    }

    file.close();

    return ok();
}

/**
 * @brief Create a file
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto CreateFile(String path) -> ErrorOr<>
{
    if (LittleFS.exists(path))
        return failure({
            .context = "CreateFile",
            .message = "File already exists",
        });

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        return failure({
            .context = "CreateFile",
            .message = "Failed to create the file",
        });
    }

    file.close();

    return ok();
}

/**
 * @brief Check if a file exists
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto FileExists(String path) -> ErrorOr<>
{
    if (LittleFS.exists(path))
        return ok();

    return failure({
        .context = "FileExists",
        .message = "File does not exist",
    });
}

/**
 * @brief Open a file
 *
 * @param path The path of the file
 * @param mode The mode of the file
 *
 * @return ErrorOr<File> can be the file or failure()
 */
auto OpenFile(String path, String mode) -> ErrorOr<File>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "OpenFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, mode.c_str());

    if (!file)
    {
        return failure({
            .context = "OpenFile",
            .message = "Failed to open the file",
        });
    }

    return ok(file);
}

auto CloseFile(File &file) -> ErrorOr<>
{
    if (!file)
    {
        return failure({
            .context = "CloseFile",
            .message = "File is not open",
        });
    }

    INTERNAL_DEBUG() << "Closing file...";

    file.close();

    return ok();
}

/**
 * @brief The credentials of a WiFi network
 */
struct WiFiCredentials
{
    String ssid;
    String password;
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
        INTERNAL_DEBUG() << "Failed to connect to WiFi.";
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

/**
 * @brief Get the WiFi credentials
 *
 * @return ErrorOr<WiFiCredentials> can be the credentials or failure()
 */
auto GetWiFiCredentials() -> ErrorOr<WiFiCredentials>
{
    auto readResult = ReadFromFile("/session.txt", '\n');

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

    auto createResult = CreateFile("/session.txt");

    if (!createResult.ok())
    {
        INTERNAL_DEBUG() << createResult.error();
        auto openResult = OpenFile("/session.txt", "w");

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

/**
 * @brief Sync the WiFi credentials by file system
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SyncWiFiByFileSystem() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing WiFi by file system...";

    auto result = GetWiFiCredentials();

    if (!result.ok())
    {
        INTERNAL_DEBUG() << "Failed to get WiFi credentials: " << result.error();

        return failure({
            .context = "SyncWiFiByFileSystem",
            .message = "Failed to get WiFi credentials",
        });
    }

    WiFiCredentials credentials = result.unwrap();

    auto result2 = WiFiConnect(credentials);

    if (!result2.ok())
    {
        INTERNAL_DEBUG() << "Failed to connect to WiFi";

        return failure({
            .context = "SyncWiFiByFileSystem",
            .message = "Failed to connect to WiFi",
        });
    }

    return ok();
}

/**
 * @brief Sync the WiFi credentials by local host
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SyncWiFiByWebHost() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing WiFi by local host...";

    WiFi.mode(WIFI_AP_STA);

    WiFi.softAPConfig(
        IPAddress(192, 168, 1, 1),
        IPAddress(192, 168, 1, 1),
        IPAddress(255, 255, 255, 0));

    WiFi.softAP("Configure o sensor Naturart");

    DNSServer dnsServer;
    dnsServer.start(53, "*", IPAddress(192, 168, 1, 1));

    AsyncWebServer server(80);
    WiFiCredentials credentials;
    bool ready = false;

    server.on("/wifi", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "GET /wifi";
                  request->send(LittleFS, "/public/wifi.html", String(), false);
              });

    server.on("/wifi", HTTP_POST,
              [&](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "POST /wifi";
                  auto *ssid = request->getParam("ssid", true);
                  auto *password = request->getParam("password", true);

                  GuardArgumentCollection args = GuardArgumentCollection();

                  args.add(IGuardArgument{.any = ssid, .name = "SSID"});
                  args.add(IGuardArgument{.any = password, .name = "Password"});

                  auto result = Guard::againstNullBulk(args);

                  if (!result.succeeded)
                  {
                      INTERNAL_DEBUG() << "Guard failed: " << result.message;
                      request->send(LittleFS, "/public/wifi_error.html", String(), false);
                      return;
                  }

                  credentials = {
                      .ssid = ssid->value(),
                      .password = password->value(),
                  };

                  request->send(200, "text/plain", "OK");
                  ready = true;
              });

    server.begin();

    while (!ready)
    {
        dnsServer.processNextRequest();
        delay(10);
    }

    server.end();

    auto result = WiFiConnect(credentials);

    if (!result.ok())
    {
        INTERNAL_DEBUG() << "Failed to connect to WiFi";

        return failure({
            .context = "SyncWiFiByWebHost",
            .message = "Failed to connect to WiFi",
        });
    }

    auto saveResult = SaveWiFiCredentials(credentials);

    if (!saveResult.ok())
    {
        INTERNAL_DEBUG() << saveResult.error();
        return failure({
            .context = "SyncWiFiByWebHost",
            .message = "Failed to save WiFi credentials",
        });
    }

    return ok();
}

/**
 * @brief Sync the WiFi credentials
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SyncWiFi() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing WiFi...";

    auto result = SyncWiFiByFileSystem();

    if (result.ok())
    {
        INTERNAL_DEBUG() << "Synced by file system";
        return ok();
    }

    INTERNAL_DEBUG() << result.error();

    auto result2 = SyncWiFiByWebHost();

    if (result2.ok())
    {
        INTERNAL_DEBUG() << "Synced by local host";
        return ok();
    }

    INTERNAL_DEBUG() << result.error();

    return failure({
        .context = "SyncWiFi",
        .message = "Failed to sync WiFi",
    });
}



void setup()
{
    Serial.begin(9600);

    if (!LittleFS.begin())
    {
        INTERNAL_DEBUG() << "Failed to mount file system";
        return;
    }

    auto result = SyncWiFi();

    if (!result.ok())
    {
        INTERNAL_DEBUG() << result.error();
        return;
    }
}

void loop()
{
    delay(0);
}