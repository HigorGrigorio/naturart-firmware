/**
 * @file sync-sensor-credentials.h
 * @brief Syncs the sensor credentials.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2021-07-10
 *
 */

#ifndef _SyncSensorCredentials_h
#define _SyncSensorCredentials_h

#include <get-sensor-credentials-from-broker.h>
#include <get-user-entry.h>

/**
 * @brief Sync the sensor credentials
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SyncSensor() -> ErrorOr<>
{
    if (IsEmptyFile(ENTRY_FILE))
    {
        // This function will never return, because the ESP8266 will be restarted by the web server.
        GetUserEntryFromUser();
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

    UserEntry userEntry = entryResult.unwrap();

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

#endif // ! _SyncSensorCredentials_h