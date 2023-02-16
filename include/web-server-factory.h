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

#include <Guard.h>

#include <ESPAsyncWebServer.h>

auto makeWebServerBase() -> AsyncWebServer
{
    return AsyncWebServer(80);
}

auto makeWebServerToWifiConfig(AsyncWebServer &server) -> void
{
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

                  WiFiCredentials wifiCredentials = {
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
}

#endif // ! _WebServerFactory_h_