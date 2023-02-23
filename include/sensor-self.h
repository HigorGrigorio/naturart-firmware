/**
 * @file sensor-self.h
 * @brief Sensor self.
 * @details This file contains the functions to get the sensor id and credentials from the broker.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-23-2
 *
 */

#ifndef _SensorSelf_h
#define _SensorSelf_h

#include <config/file-system.h>
#include <file.h>

#include <StringHelper.h>

auto SelfFromBrokerPayload(String &payload) -> ErrorOr<String>
{
    INTERNAL_DEBUG() << "Parsing the payload: " << payload;

    ErrorOr<String> result;

    if (payload.length() == 0)
    {
        result = failure({.context = "GetSensorSelfFromBrokerPayload",
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
                auto array2 = successResult.unwrap();

                if (!array2.at(1)->equals("true"))
                {
                    result = failure({
                        .context = "GetSensorSelfFromBrokerPayload",
                        .message = "The payload is not valid",
                    });
                }
                else
                {
                    INTERNAL_DEBUG() << "Parsing the payload...";

                    auto result2 = utility::StringHelper::splitStringToArray(*array.at(1), '=');

                    if (!result2.ok())
                    {
                        result = failure(result2.error());
                    }
                    else
                    {
                        auto id = *result2.unwrap().at(1);

                        INTERNAL_DEBUG() << "Sensor id: " << id;

                        result = ok(id);
                    }
                }
            }
        }
    }

    return result;
}

/**
 * @brief Get the sensor id from the file system.
 *
 * @param id The sensor id.
 *
 * @return ErrorOr<String> The sensor id.
 */
auto SaveSelf(String &id) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Saving the sensor id...";

    ErrorOr<> result;

    auto file = OpenFile(SELF_FILE, "w");

    if (!file.ok())
    {
        result = file.error();
    }
    else
    {
        auto file1 = file.unwrap();

        file1.println(id);

        file1.close();

        result = ok();
    }

    return result;
}

auto LoadSelf() -> ErrorOr<String>
{
    INTERNAL_DEBUG() << "Loading the sensor id...";
    ErrorOr<String> result;

    if (IsEmptyFile(SELF_FILE))
    {
        result = failure({
            .context = "LoadSelf",
            .message = "The sensor id file is empty",
        });
    }
    else
    {
        auto readResult = ReadFromFile(SELF_FILE, '\n');

        if (!readResult.ok())
        {
            result = readResult.error();
        }
        else
        {
            auto lines = readResult.unwrap();
            auto id = lines.at(0)->substring(0, lines.at(0)->length() - 1);

            result = ok(id);
        }
    }
    return result;
}

#endif // ! _SensorSelf_h