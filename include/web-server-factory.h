/**
 * @file web-server-factory.h
 * @brief Web server factory
 * @details This file contains the functions factory to AsyncWebServer class.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2020-05-01
 *
 */

#ifndef _WebServerFactory_h_
#define _WebServerFactory_h_

#include <wifi-credentials.h>
#include <user-entry.h>

#include <Guard.h>

#include <ESPAsyncWebServer.h>

#ifndef WEB_SERVER_LOCAL_IP
#warning "WEB_SERVER_LOCAL_IP is not defined"
#define WEB_SERVER_LOCAL_IP IPAddress(192, 168, 1, 1)
#endif

#ifndef WEB_SERVER_GATEWAY
#warning "WEB_SERVER_GATEWAY is not defined"
#define WEB_SERVER_GATEWAY IPAddress(192, 168, 1, 1)
#endif

#ifndef WEB_SERVER_SUBNET
#warning "WEB_SERVER_SUBNET is not defined"
#define WEB_SERVER_SUBNET IPAddress(255, 255, 255, 0)
#endif

#ifndef WEB_SERVER_AP_SSID
#warning "WEB_SERVER_AP_SSID is not defined"
#define WEB_SERVER_AP_SSID "Configure o sensor Naturart"
#endif

/**
 * @brief Configure the WiFi to be a web server
 */
auto ConfigureWiFiToWebServer(IPAddress localIP = WEB_SERVER_LOCAL_IP,
                              IPAddress getway = WEB_SERVER_GATEWAY,
                              IPAddress subnet = WEB_SERVER_SUBNET) -> void
{
    WiFi.mode(WIFI_AP_STA);

    WiFi.softAPConfig(
        localIP,
        getway,
        subnet);

    WiFi.softAP(WEB_SERVER_AP_SSID);

    WiFi.setSleep(false);
}

auto MakeWebServerBase() -> AsyncWebServer
{
    auto server = AsyncWebServer(80);

    server.on("/shared/style.css", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "GET /style.css";
                  request->send(LittleFS, "/public/shared/style.css", "text/css", false);
              });

    server.on("/shared/index.js", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "GET /index.js";
                  request->send(LittleFS, "/public/shared/index.js", "text/script", false);
              });

    return server;
}

auto ConstructWebServerToWifiConfig(AsyncWebServer &server) -> void
{
    server.on("/", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "GET /";
                  request->send(LittleFS, "/public/wifi/index.html", "text/html", false);
              });

    server.on("/index.js", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "GET /index.js";
                  request->send(LittleFS, "/public/wifi/index.js", "test/script", false);
              });

    server.on("/", HTTP_POST,
              [&](AsyncWebServerRequest *request)
              {
                  INTERNAL_DEBUG() << "POST /";
                  auto *ssid = request->getParam("ssid", true);
                  auto *password = request->getParam("password", true);

                  GuardArgumentCollection args = GuardArgumentCollection();

                  args.add(IGuardArgument{.any = ssid, .name = "SSID"});
                  args.add(IGuardArgument{.any = password, .name = "Password"});

                  auto result = Guard::againstNullBulk(args);

                  if (!result.succeeded)
                  {
                      INTERNAL_DEBUG() << "Guard failed: " << result.message;
                      request->send(400);
                      return;
                  }

                  WiFiCredentials wifiCredentials = {
                      .ssid = ssid->value(),
                      .password = password->value(),
                  };

                  auto saveResult = SaveWiFiCredentials(wifiCredentials);

                  if (!saveResult.ok())
                  {
                      INTERNAL_DEBUG() << "Failed to save WiFi credentials: " << saveResult.error();
                      request->send(422);
                      return;
                  }

                  request->send(200);
                  ESP.restart();
              });
}

auto ContructWebServerToUserCredentialsConfig(AsyncWebServer &server) -> void
{
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

                  UserEntry userEntry = {
                      .name = username->value(),
                      .password = password->value(),
                      .serialCode = serialCode->value(),
                      .cpf = cpf->value(),
                  };

                  auto result1 = SaveUserEntry(userEntry);

                  // TODO: Send a response to the client
                  if (result1.ok())
                  {
                      request->send(200, "text/plain", "OK");
                      ESP.restart();
                  }
              });
}

#endif // ! _WebServerFactory_h_