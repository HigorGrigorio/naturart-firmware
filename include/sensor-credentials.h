/**
 * @file sensor-credentials.h
 * @brief Sensor credentials
 * @details This file contains declarations and functions for manipuling the credentials of the sensors.
 * @version 1.0
 * @date 2021-04-15
 *
 */

#ifndef _SensorCredentials_h_
#define _SensorCredentials_h_

#include <file.h>
#include <config/file-sistem.h>

#include <ArduinoJson.h>

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

auto CreadentialsFromJson(String json) -> ErrorOr<SensorCredentials>
{
    ErrorOr<SensorCredentials> result;

    auto credentials = SensorCredentials();
    auto doc = DynamicJsonDocument(json.length() + 1);

    auto err = deserializeJson(doc, json);

    if (err != DeserializationError::Ok)
    {
        result = failure({
            .context = "creadentialsFromJson",
            .message = err.c_str(),
        });
    }
    else if (doc.containsKey("success") && doc["success"].as<bool>())
    {
        if (!doc.containsKey("body"))
        {
            result = failure({
                .context = "creadentialsFromJson",
                .message = "The body is missing",
            });
        }
        else
        {
            auto list = SensorCredentials();

            for (auto item : doc["body"].as<JsonArray>())
            {
                auto type = item["type"].as<String>();
                auto id = item["id"].as<String>();

                list.add({type, id});
            }

            result = ok(list);
        }
    }
    else
    {
        result = failure({
            .context = "creadentialsFromJson",
            .message = (doc.containsKey("message") ? doc["message"].as<const char *>() : "Unknown error"),
        });
    }

    return result;
}

auto GetSensorCredentials() -> ErrorOr<SensorCredentials>
{
    auto result = ErrorOr<SensorCredentials>();

    if (!FileExists(SELF_FILE))
    {
        result = failure({
            .context = "GetSensorCredentials",
            .message = "The file does not exists",
        });
    }
    else
    {
        // TODO: read the file
    }

    return result;
}

#endif // ! _SensorCredentials_h_