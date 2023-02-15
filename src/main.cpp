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

#include <PubSubClient.h>

#include <ErrorOr.h>
#include <Guard.h>
#include <Nonnull.h>
#include <Check.h>
#include <UtilStringArray.h>

#include <wifi-connection.h>

/**
 * @brief The credential of a sensor
 */
struct SensorType
{
    String type = "";
    String id = "";
};

/**
 * @brief The group of credentials of a sensor
 */
using SensorCredentials = LL<struct SensorType>;

/**
 * @brief The credentials of a user
 */
struct UserEntry
{
    String id = "";
    String name = "";
    String password = "";
    String serialCode = "";
    String cpf = "";
};

/**
 * @brief Convert a UserEntry to JSON
 */
auto toJson(UserEntry &entry) -> String
{
    return String("{") +
           "\"id\":\"" + entry.id + "\"," +
           "\"name\":\"" + entry.name + "\"," +
           "\"password\":\"" + entry.password + "\"," +
           "\"serialCode\":\"" + entry.serialCode + "\"," +
           "\"cpf\":\"" + entry.cpf + "\"" +
           "}";
}

/**
 * STATIC VARIABLES
 */

// The server
static AsyncWebServer server(80);

// The DNS server
static DNSServer dnsServer;

// The WiFi client
static WiFiClient wifiClient;

// The MQTT client
static PubSubClient mqttClient(wifiClient);

// An helper to sync the WiFi by the web host
static bool syncWiFiByWebHostHelper = false;

// The WiFi credentials
static WiFiCredentials wifiCredentials;

// The user credentials
static UserEntry userEntry;

// The sensor credentials
static SensorCredentials sensorCredentials;

// The sync helper. It's used to sync the sensor credentials by the web host.
static bool syncSensorCredentialsHelper = false;

// The local IP.
static IPAddress localIP(192, 168, 1, 1);

// Guard if the server is initialized.
static bool serverInitialized = false;

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
 * @return bool true if the file exists, false otherwise
 */
auto FileExists(String path) -> bool
{
    return LittleFS.exists(path);
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

auto IsEmptyFile(String path) -> bool
{
    auto file = LittleFS.open(path, "r");
    bool empty = true;

    if (file)
    {
        empty = file.size() == 0;
        file.close();
    }

    return empty;
}



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

auto GetUserEntry() -> ErrorOr<UserEntry>
{
    auto readResult = ReadFromFile(ENTRY_FILE, '\n');

    if (!readResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to read the session file.";
        return failure(readResult.error());
    }

    // unwrap the lines of file.
    auto lines = readResult.unwrap();

    // check if the file has 4 lines.
    if (lines.length() != 4)
    {
        INTERNAL_DEBUG() << "The session file has not 4 lines.";
        return failure({
            .context = "GetUserEntry",
            .message = "The session file has not 4 lines",
        });
    }

    auto first = *lines.at(0);
    auto second = *lines.at(1);
    auto third = *lines.at(2);
    auto fourth = *lines.at(3);

    // remove the last character of the lines.
    return ok<UserEntry>({
        .name = second.substring(0, second.length() - 1),
        .password = third.substring(0, third.length() - 1),
        .serialCode = fourth.substring(0, fourth.length() - 1),
        .cpf = first.substring(0, first.length() - 1),
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
        return failure({
            .context = "SyncWiFiByFileSystem",
            .message = "Failed to connect to WiFi",
        });
    }

    return ok();
}

/**
 * @brief Turn on the built-in LED
 */
auto TurnOnBuiltInLed() -> void
{
    digitalWrite(LED_BUILTIN, HIGH);
}

/**
 * @brief Turn off the built-in LED
 */
auto TurnOffBuiltInLed() -> void
{
    digitalWrite(LED_BUILTIN, LOW);
}

auto SaveUserEntry(UserEntry &entry) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Saving user entry...";

    if (!FileExists(ENTRY_FILE))
    {
        CreateFile(ENTRY_FILE);
    }

    auto openResult = OpenFile(ENTRY_FILE, "w");

    if (!openResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to open the file.";
        return failure(openResult.error());
    }

    File file = openResult.unwrap();

    file.println(entry.cpf);
    file.println(entry.name);
    file.println(entry.password);
    file.println(entry.serialCode);

    return ok();
}

/**
 * @brief Initialize the web server.
 */
auto InitWebServer() -> ErrorOr<>
{
    if (!serverInitialized)
    {
        INTERNAL_DEBUG() << "Initializing web server...";

        IPAddress ip(192, 168, 1, 1);

        WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));

        WiFi.softAP("Configure o sensor Naturart");

        // init web server
        dnsServer.start(53, "*", ip);

        INTERNAL_DEBUG() << "Starting web server...";
        INTERNAL_DEBUG() << "IP address: " << ip.toString();

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

                      wifiCredentials = {
                          .ssid = ssid->value(),
                          .password = password->value(),
                      };

                      auto saveResult = SaveWiFiCredentials(wifiCredentials);

                      if (!saveResult.ok())
                      {
                          INTERNAL_DEBUG() << "Failed to save WiFi credentials: " << saveResult.error();
                          request->send(LittleFS, "/public/wifi_error.html", String(), false);
                          return;
                      }

                      // TODO: Send a response to the client
                      request->send(200, "text/plain", "OK");
                      ESP.restart();
                  });

        server.on("/sync", HTTP_GET,
                  [](AsyncWebServerRequest *request)
                  {
                      INTERNAL_DEBUG() << "GET /sync";
                      request->send(LittleFS, "/public/sensor.html", String(), false);
                  });

        server.on("/sync", HTTP_POST,
                  [&](AsyncWebServerRequest *request)
                  {
                      INTERNAL_DEBUG() << "POST /sync";
                      auto *username = request->getParam("username", true);
                      auto *password = request->getParam("password", true);
                      auto *cpf = request->getParam("cpf", true);
                      auto *serialCode = request->getParam("serialCode", true);

                      GuardArgumentCollection args = GuardArgumentCollection();

                      args.add(IGuardArgument{.any = username, .name = "Username"});
                      args.add(IGuardArgument{.any = password, .name = "Password"});
                      args.add(IGuardArgument{.any = cpf, .name = "CPF"});
                      args.add(IGuardArgument{.any = serialCode, .name = "Serial Core"});

                      auto result = Guard::againstNullBulk(args);

                      if (!result.succeeded)
                      {
                          INTERNAL_DEBUG() << "Guard failed: " << result.message;
                          request->send(206, "text/plain", "Conteudos parciais");
                          return;
                      }

                      userEntry = {
                          .name = username->value(),
                          .password = password->value(),
                          .serialCode = serialCode->value(),
                          .cpf = cpf->value(),
                      };

                      auto result1 = SaveUserEntry(userEntry);

                      if (result1.ok())
                      {
                          request->send(200, "text/plain", "OK");
                          ESP.restart();
                      }
                  });

        serverInitialized = true;
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

    // Need initialization of server.
    auto result = InitWebServer();

    if (!result.ok())
    {
        INTERNAL_DEBUG() << result.error();
        return failure({
            .context = "SyncWiFiByWebHost",
            .message = "Failed to init the web server",
        });
    }

    server.begin();
    TurnOnBuiltInLed();

    INTERNAL_DEBUG() << "Server started. Waiting for WiFi credentials...";

    while (!syncWiFiByWebHostHelper)
    {
        dnsServer.processNextRequest();
        delay(10);
    }

    server.end();
    TurnOffBuiltInLed();

    syncWiFiByWebHostHelper = false;

    INTERNAL_DEBUG() << "WiFi credentials received. Connecting to WiFi...";

    auto result1 = WiFiConnect(wifiCredentials);

    if (!result1.ok())
    {
        INTERNAL_DEBUG() << "Failed to connect to WiFi";

        return failure({
            .context = "SyncWiFiByWebHost",
            .message = "Failed to connect to WiFi",
        });
    }

    auto saveResult = SaveWiFiCredentials(wifiCredentials);

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

    mqttClient.publish("sync", toJson(entry).c_str());

    while (!receive)
    {
        delay(10);
    }

    INTERNAL_DEBUG() << "Received";
    return ok<SensorCredentials>({});
}

auto SyncSensorFromNaturartServer() -> ErrorOr<SensorCredentials>
{
    INTERNAL_DEBUG() << "Syncing sensor credentials from Naturart server...";

    // Requires WiFi to be disconnected to avoid conflicts with the web server.
    if (WiFi.isConnected())
    {
        WiFi.disconnect();
    }

    auto result = InitWebServer();

    if (!result.ok())
    {
        INTERNAL_DEBUG() << result.error();
        return failure({
            .context = "SyncSensorCredentialsFromNaturartServer",
            .message = "Failed to init the web server",
        });
    }

    server.begin();
    TurnOnBuiltInLed();

    // TODO: add a timeout.
    // await for the user entry.
    while (!syncSensorCredentialsHelper)
    {
        dnsServer.processNextRequest();
        delay(10);
    }

    server.end();
    TurnOffBuiltInLed();

    // to evit conflits.
    syncSensorCredentialsHelper = false;

    // needs validate the user entry.
    auto result1 = SyncWiFiByFileSystem();

    if (!result1.ok())
    {
        INTERNAL_DEBUG() << result1.error();
        return failure({
            .context = "SyncSensorCredentialsFromNaturartServer",
            .message = "Failed to sync WiFi by file system",
        });
    }

    // send the entry for web broker.
    auto result2 = GetSensorCredentialsFromBroker(userEntry);

    if (!result2.ok())
    {
        INTERNAL_DEBUG() << result2.error();
        return failure({
            .context = "GetSensorCredentialsFromNaturartServer",
            .message = "Failed to get sensor credentials",
        });
    }

    sensorCredentials = result2.unwrap();

    return ok(sensorCredentials);
}

/**
 * @brief Sync the sensor credentials
 */
auto SyncSensor() -> ErrorOr<>
{
    if (IsEmptyFile(ENTRY_FILE))
    {
        SyncSensorFromNaturartServer();
    }

    auto entryResult = GetUserEntry();

    if (!entryResult.ok())
    {
        INTERNAL_DEBUG() << entryResult.error();
        return failure({
            .context = "SyncSensor",
            .message = "Failed to get user entry",
        });
    }

    userEntry = entryResult.unwrap();

    auto result = GetSensorCredentialsFromBroker(userEntry);

    if (!result.ok())
    {
        INTERNAL_DEBUG() << result.error();
        return failure({
            .context = "SyncSensor",
            .message = "Failed to get sensor credentials",
        });
    }

    return ok();
}

void setup()
{
    Serial.begin(9600);

    WiFi.mode(WIFI_AP_STA);

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
}

void loop()
{
    delay(0);
}