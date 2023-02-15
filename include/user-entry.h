/**
 * @file user-entry.h
 * @brief User entry
 * @details This file contains declarations and functions for manipuling the user entry.
 * @version 1.0
 * @date 2021-04-15
 *
 */

#ifndef _UserEntry_h_
#define _UserEntry_h_

#include <file.h>
#include <config/file-sistem.h>

/**
 * @brief The credentials of a user
 */
struct UserEntry
{
    String id = "";
    String name = "";
    String password = "";
    String serialCode = "";
    String cpf = "";

    /**
     * @brief Convertes the user entry to a JSON string
     *
     * @return The JSON string
     */
    auto ToJson() -> String
    {
        return String("UserEntry {") +
               "id: " + id + ", " +
               "name: " + name + ", " +
               "password: " + password + ", " +
               "serialCode: " + serialCode + ", " +
               "cpf: " + cpf +
               "}";
    }
};

/**
 * @brief Reads the user entry from the file
*/
auto GetUserEntry() -> ErrorOr<UserEntry>
{
    auto readResult = ReadFromFile(ENTRY_FILE, '\n');

    if (!readResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to read the session file.";
        return failure(readResult.error());
    }

    // unwrap the lines of file.
    auto lines = readResult.unwrap();

    // check if the file has 4 lines.
    if (lines.length() != 4)
    {
        INTERNAL_DEBUG() << "The session file has not 4 lines.";
        return failure({
            .context = "GetUserEntry",
            .message = "The session file has not 4 lines",
        });
    }

    auto first = *lines.at(0);
    auto second = *lines.at(1);
    auto third = *lines.at(2);
    auto fourth = *lines.at(3);

    // remove the last character of the lines.
    return ok<UserEntry>({
        .name = second.substring(0, second.length() - 1),
        .password = third.substring(0, third.length() - 1),
        .serialCode = fourth.substring(0, fourth.length() - 1),
        .cpf = first.substring(0, first.length() - 1),
    });
}

/**
 * @brief Saves the user entry to the file
*/
auto SaveUserEntry(UserEntry &entry) -> ErrorOr<>
{
    INTERNAL_DEBUG() << "Saving user entry...";

    if (!FileExists(ENTRY_FILE))
    {
        CreateFile(ENTRY_FILE);
    }

    auto openResult = OpenFile(ENTRY_FILE, "w");

    if (!openResult.ok())
    {
        INTERNAL_DEBUG() << "Failed to open the file.";
        return failure(openResult.error());
    }

    File file = openResult.unwrap();

    file.println(entry.cpf);
    file.println(entry.name);
    file.println(entry.password);
    file.println(entry.serialCode);

    return ok();
}

#endif // ! _UserEntry_h_