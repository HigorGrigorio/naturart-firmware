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
            INTERNAL_DEBUG() << "Splitting the payload...";
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

                        if (result2.ok())
                        {
                            auto array2 = result2.unwrap();

                            if (array2.length() == 0)
                            {
                                continue;
                            }

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
        auto openResult = OpenFile(SELF_FILE, "r");

        if (!openResult.ok())
        {
            result = failure(openResult.error());
        }
        else
        {
            File file = openResult.unwrap();

            if (file)
            {
                String type;
                String id;
                SensorCredentials credentials;

                auto readResult = ReadFromFile(SELF_FILE, '\n');

                if (!readResult.ok())
                {
                    result = failure(readResult.error());
                }
                else
                {
                    auto array = readResult.unwrap();

                    int i = 0;

                    for (; i < array.length(); i++)
                    {
                        // odd
                        if ((i & 1) == 0)
                        {
                            id = *array.at(i);
                            credentials.add({.type = type, .id = id});
                        }
                        else
                        {
                            type = *array.at(i);
                        }
                    }

                    // odd values ​​represent unformed pairs
                    if (i & 1)
                    {
                        // clean file for read new values.
                        CleanFile(SELF_FILE);
                        CleanFile(ENTRY_FILE);
                    }
                }

                result = ok(credentials);
            }
            else
            {
                result = failure({
                    .context = "GetSensorCredentials",
                    .message = "Opening the file resulted in an error",
                });
            }

            file.close();
        }
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
            for (auto &credential : credentials)
            {
                INTERNAL_DEBUG() << "Saving credential: " << credential.type << " - " << credential.id;

                file.println(credential.type);
                file.println(credential.id);
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