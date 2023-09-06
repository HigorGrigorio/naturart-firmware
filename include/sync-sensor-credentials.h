/**
 * @file sync-sensor-credentials.h
 * @brief Syncs the sensor credentials.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 *
 */

#ifndef _SyncSensorCredentials_h
#define _SyncSensorCredentials_h

#include <get-sensor-credentials-from-broker.h>
#include <get-sensor-id-from-broker.h>
#include <get-user-entry.h>

/**
 * @brief Sync the sensor credentials
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SyncSensor() -> ErrorOr<>
{
    ErrorOr<> result = ok();

    if (IsEmptyFile(TYPING_FILE))
    {
        if (IsEmptyFile(ENTRY_FILE))
        {
            // This function will never return, because the ESP8266 will be restarted by the web server.
            GetUserEntryFromWebServer();
        }

        auto entryResult = GetUserEntry();

        if (!entryResult.ok())
        {
            INTERNAL_DEBUG() << entryResult.error();

            result = failure({
                .context = "SyncSensor",
                .message = "Failed to get user entry",
            });
        }
        else
        {
            UserEntry userEntry = entryResult.unwrap();

            if (IsEmptyFile(SELF_FILE))
            {
                GetSensorIdFromBroker(userEntry);
            }

            auto selfResult = LoadSelf();

            if (!selfResult.ok())
            {
                INTERNAL_DEBUG() << selfResult.error();

                result = failure({
                    .context = "SyncSensor",
                    .message = "Failed to get sensor id",
                });
            }
            else
            {
                auto id = selfResult.unwrap();

                if (IsEmptyFile(TYPING_FILE))
                {
                    GetSensorCredentialsFromBroker(id);
                }

                if (!selfResult.ok())
                {
                    INTERNAL_DEBUG() << selfResult.error();

                    result = failure({
                        .context = "SyncSensor",
                        .message = "Failed to get sensor credentials",
                    });
                }
            }
        }
    }

    return result;
}

#endif // ! _SyncSensorCredentials_h