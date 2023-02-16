/**
 * @file main.cpp
 * @brief The main file of the project
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 */

#include <config/system.h>

#include <Arduino.h>
#include <LittleFS.h>

#include <DNSServer.h>

#include <PubSubClient.h>

#include <sensor-credentials.h>
#include <wifi-credentials.h>
#include <user-entry.h>
#include <web-server-factory.h>
#include <blink-led.h>

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
    auto server = makeWebServerBase();

    makeWebServerToWifiConfig(server);

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

    mqttClient.publish("sync", entry.ToJson().c_str());

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