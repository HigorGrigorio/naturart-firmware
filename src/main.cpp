/**
 * @file main.cpp
 * @brief The main file of the project
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 */

#include <config/system.h>
#include <config/dns-server.h>
#include <config/file-sistem.h>
#include <config/web-server.h>

#include <Arduino.h>
#include <LittleFS.h>

#include <DNSServer.h>

#include <PubSubClient.h>

#include <sensor-credentials.h>
#include <wifi-credentials.h>
#include <user-entry.h>
#include <web-server-factory.h>
#include <blink-led.h>
#include <dns-server-factory.h>

/**
 * STATIC VARIABLES
 */

// The WiFi client
static WiFiClient wifiClient;

// The MQTT client
static PubSubClient mqttClient(wifiClient);

// The WiFi credentials
static WiFiCredentials wifiCredentials;

// The user credentials
static UserEntry userEntry;

// The sensor credentials
static SensorCredentials sensorCredentials;

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

    auto result2 = GetWiFiCredentilasFromUser();

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

auto GetSensorCredentialsFromUser() -> ErrorOr<>
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

/**
 * @brief Sync the sensor credentials
 */
auto SyncSensor() -> ErrorOr<>
{
    if (IsEmptyFile(ENTRY_FILE))
    {
        // This function will never return, because the ESP8266 will be restarted by the web server.
        GetSensorCredentialsFromUser();
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