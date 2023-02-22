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

#include <StringHelper.h>

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

auto ToString(SensorType &sensorType) -> String
{
    return "{\"type\":\"" + sensorType.type + ",\"id\":\"" + sensorType.id + "\"}";
}

auto CredentialsToJson(SensorCredentials &credentials) -> String
{
    String buff = "[";

    for (auto it = credentials.begin(), end = credentials.end(); it != end;)
    {
        auto value = *it;

        auto parsedValue = ToString(value);

        buff += parsedValue;

        ++it;

        if (it != end)
        {
            buff += ", ";
        }
    }

    buff += ']';

    return buff;
}

auto CredentialsFromBrokerPayload(String &payload) -> ErrorOr<SensorCredentials>
{
    INTERNAL_DEBUG() << "Parsing the payload: " << payload;

    ErrorOr<SensorCredentials> result;

    if (payload.length() == 0)
    {
        result = failure({.context = "CredentialsFromBrokerPayload",
                          .message = "Empty payload"});
    }
    else
    {
        auto result1 = utility::StringHelper::splitStringToArray(payload, ';');

        if (!result1.ok())
        {
            result = failure(result1.error());
        }
        else
        {
            INTERNAL_DEBUG() << "Splitting the payload: ;";
            auto array = result1.unwrap();

            auto successResult = utility::StringHelper::splitStringToArray(*array.at(0), '=');

            if (!successResult.ok())
            {
                result = failure(successResult.error());
            }
            else
            {
                auto success = successResult.unwrap().at(1);

                INTERNAL_DEBUG() << "Success: " << *success << " (" << success->equals("true") << ")";

                if (!success->equals("true"))
                {
                    result = failure({
                        .context = "CredentialsFromBrokerPayload",
                        .message = "The payload is not valid",
                    });
                }
                else
                {
                    INTERNAL_DEBUG() << "Is a success. Parsing credentials...";

                    SensorCredentials credentials;
                    bool fail = false;

                    for (int i = 1; i < array.length(); i++)
                    {
                        auto result2 = utility::StringHelper::splitStringToArray(*array.at(i), '=');
                        INTERNAL_DEBUG() << "Value: " << *array.at(i) << " (" << result2.ok() << ")";

                        if (!result2.ok())
                        {
                            result = failure(result2.error());
                            fail = true;
                            break;
                        }
                        else
                        {
                            auto array2 = result2.unwrap();

                            auto type = array2.at(0);
                            auto id = array2.at(1);

                            INTERNAL_DEBUG() << "Adding credential: " << *type << " - " << *id;

                            credentials.add({.type = *type, .id = *id});
                        }

                        if (!fail)
                        {
                            result = ok(credentials);
                        }
                    }
                }
            }
        }
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

auto SaveSensorCredentials(SensorCredentials credentials) -> ErrorOr<>
{
    ErrorOr<> result = ok();

    INTERNAL_DEBUG()
        << "Saving sensor credentials";

    if (!FileExists(SELF_FILE))
    {
        CreateFile(SELF_FILE);
    }

    auto openResult = OpenFile(SELF_FILE, "w");

    if (!openResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to open file for writing";
        result = failure(openResult.error());
    }
    else
    {
        File file = openResult.unwrap();

        if (file)
        {
            if (!file.println(CredentialsToJson(credentials)))
            {
                result = failure({.context = "SaveSensorCredentials", .message = "Writing the file resulted in an error"});
            }
        }
        else
        {
            result = failure({.context = "SaveSensorCredentials", .message = "Opening the file resulted in an error"});
        }

        file.close();
    }

    return result;
}

#endif // ! _SensorCredentials_h_