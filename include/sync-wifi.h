/**
 * @file sync-wifi.h
 * @brief Syncs the WiFi credentials.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 *
 */

#ifndef _SyncWiFi_h
#define _SyncWiFi_h

#include <sync-wifi-by-file-system.h>
#include <sync-wifi-by-web-host.h>

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

    auto result2 = GetWiFiCredentialsFromUser();

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

#endif // ! _SyncWiFi_h