/**
 * @file sync-wifi-by-file-system.h
 * @brief Syncs the WiFi credentials by system files.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2021-07-10
 *
 */

#ifndef _SyncWiFiByFileSystem_h
#define _SyncWiFiByFileSystem_h

#include <file.h>
#include <wifi-connection.h>
#include <wifi-credentials.h>

/**
 * @brief Sync the WiFi credentials by file system
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto SyncWiFiByFileSystem() -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Syncing WiFi by file system...";

    auto result = GetWiFiCredentials();

    if (!result.ok())
    {
        INTERNAL_DEBUG() << "Failed to get WiFi credentials: " << result.error();

        return failure({
            .context = "SyncWiFiByFileSystem",
            .message = "Failed to get WiFi credentials",
        });
    }

    // When this file is empty, ESP does not connect to any wifi,
    // as it will restart when capturing data through the host
    if (!IsEmptyFile(ENTRY_FILE))
    {
        WiFiCredentials credentials = result.unwrap();

        auto result2 = WiFiConnect(credentials);

        if (!result2.ok())
        {
            return failure({
                .context = "SyncWiFiByFileSystem",
                .message = "Failed to connect to WiFi",
            });
        }
    }

    return ok();
}

#endif // ! _SyncWiFiByFileSystem_h