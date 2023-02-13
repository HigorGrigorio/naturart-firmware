#include <config/global.h>

#include <Arduino.h>

#include <core/common/Nonnull.h>
#include <core/common/Check.h>

#include <core/common/Guard.h>

#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

#include <LittleFS.h>

// Current server manager
AsyncWebServer *server = nullptr;

// DNS Server
DNSServer *dnsServer = nullptr;

bool webServerRequestHelper = false;

struct WiFiSession
{
    String ssid;
    String password;
} *session = nullptr;

auto FreePtr(void **ptr) -> void
{
    free(*ptr);
    *ptr = nullptr;
}

auto SetupWiFiSyncWebServer() -> Nonnull<AsyncWebServer *>
{
    INTERNAL_DEBUG() << "Setting up WiFi Sync Web Server...";
    server = new AsyncWebServer(80);
    server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        INTERNAL_DEBUG() << "GET /wifi";

        request->send(LittleFS, "/public/wifi.html", String(), false); });

    server->on("/wifi", HTTP_POST,
               [](AsyncWebServerRequest *request)
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

                   session = new WiFiSession();

                   session->ssid = ssid->value();
                   session->password = password->value();

                   INTERNAL_DEBUG()
                       << "SSID: "
                       << session->ssid
                       << " Password: "
                       << session->password;

                   webServerRequestHelper = true;

                   request->send(200, "text/plain", "Sucesso!");
               });

    webServerRequestHelper = false;
    server->begin();

    digitalWrite(LED_BUILTIN, HIGH);

    return server;
}

auto EndServer(Nonnull<AsyncWebServer *> server) -> void
{
    if (server != nullptr)
    {
        server->end();
        delete server;
        server = nullptr;
        digitalWrite(LED_BUILTIN, LOW);
    }
}

auto SetupDNSServer() -> Nonnull<DNSServer *>
{
    INTERNAL_DEBUG() << "Setting up DNS Server...";

    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP("Naturart Sensor Config", "");

    DNSServer *dnsServer = new DNSServer();
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer->start(53, "*", local_ip);

    return dnsServer;
}

auto SetupFileSystem() -> bool
{
    INTERNAL_DEBUG() << "Setting up file system...";
    if (!LittleFS.begin())
    {
        INTERNAL_DEBUG() << "An Error has occurred while mounting LittleFS";
        return false;
    }

    return true;
}

auto SetupSerial() -> bool
{
    Serial.begin(DEFAULT_SERIAL_BAUD_RATE);
    while (!Serial)
        ;
    return true;
}

auto SyncWiFiByFileSystem() -> ErrorOr<bool>
{
    INTERNAL_DEBUG() << "Syncing WiFi by file system...";

    if (LittleFS.exists(SESSION_FILE))
    {
        if (File sessionFile = LittleFS.open(SESSION_FILE, "r"))
        {
            String buff = "";

            if (!session)
                session = new WiFiSession();

            int i = 0;
            while (sessionFile.available() > 0)
            {
                char c = (char)sessionFile.read();

                if (c == '\n')
                {
                    buff = buff.substring(0, buff.length() - 1);

                    if (i == 0)
                    {
                        INTERNAL_DEBUG() << "Buff: " << buff;
                        session->ssid = buff;
                    }
                    else if (i == 1)
                    {
                        INTERNAL_DEBUG() << "Buff: " << buff;
                        session->password = buff;
                        break;
                    }
                    i++;
                    buff = "";
                    continue;
                }
                buff += c;
            }

            if (session->ssid.length() == 0 || session->password.length() == 0)
            {
                return failure({.message = "Invalid session file",
                                .context = "SyncWiFiByFileSystem"});
            }

            WiFi.begin(session->ssid.c_str(), session->password.c_str());

            sessionFile.close();

            WiFi.waitForConnectResult();

            if (WiFi.status() != WL_CONNECTED)
            {
                return failure({.message = "Failed to connect to WiFi",
                                .context = "SyncWiFiByFileSystem"});
            }

            INTERNAL_DEBUG()
                << ("WiFi connected")
                << ("IP address: ")
                << (WiFi.localIP().toString());
        }
    }
    return ok(true);
}

auto SyncWifiByWebServer() -> ErrorOr<bool>
{
    INTERNAL_DEBUG() << "Syncing WiFi by web server...";

    if (WiFi.status() == WL_CONNECTED)
    {
        INTERNAL_DEBUG() << "WiFi connected.";
        return false;
    }

    if (!dnsServer)
    {
        dnsServer = SetupDNSServer();
    }

    if (!server)
    {
        server = SetupWiFiSyncWebServer();
    }

    while (!webServerRequestHelper)
    {
        dnsServer->processNextRequest();
    }

    if (!session)
    {
        INTERNAL_DEBUG() << "Session is null.";
        return ok(false);
    }

    WiFi.begin(session->ssid.c_str(), session->password.c_str());

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        return failure({.message = "Failed to connect to WiFi",
                        .context = "SyncWifiByWebServer"});
    }

    INTERNAL_DEBUG()
        << ("WiFi connected")
        << ("IP address: ")
        << (WiFi.localIP().toString());

    // save session
    File sessionFile = LittleFS.open(SESSION_FILE, "w+");

    if (sessionFile)
    {
        INTERNAL_DEBUG() << "Saving session...";

        INTERNAL_DEBUG() << "SSID: " << session->ssid
                         << " Password: " << session->password;

        sessionFile.println(session->ssid);
        sessionFile.println(session->password);
    }

    sessionFile.close();

    EndServer(server);
    return ok(true);
}

auto SyncWiFi() -> ErrorOr<bool>
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return ok(true);
    }

    auto syncFSFileSystemResult = SyncWiFiByFileSystem();

    if (syncFSFileSystemResult.ok())
    {
        INTERNAL_DEBUG() << "Synced WiFi by file system.";
        return ok(true);
    }

    auto syncWiFiByWebServerResult = SyncWifiByWebServer();

    if (syncWiFiByWebServerResult.ok())
    {
        INTERNAL_DEBUG() << "Synced WiFi by web server.";
        return ok(true);
    }

    return WiFi.isConnected();
}

void setup()
{
    if (!SetupSerial())
        return;

    if (!SetupFileSystem())
        return;

    auto syncWiFiResult = SyncWiFi();

    if (!syncWiFiResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to sync WiFi.";
        return;
    }
}

void loop()
{
}