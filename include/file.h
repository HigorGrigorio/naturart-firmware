/**
 * @file file.h
 * @brief defines the function for manipuling files.
 * @details This file contains the function for manipuling files.
 * @version 1.0
 * @date 2021-04-15
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 *
 */

#ifndef _File_h_
#define _File_h_

#include <UtilStringArray.h>
#include <ErrorOr.h>

#include <LittleFS.h>

/**
 * @brief Write in a file
 *
 * @param path The path of the file
 * @param content The content to write
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto WriteInFile(String path, String content, bool endline) -> ErrorOr<>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "WriteInFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        return failure({
            .context = "WriteInFile",
            .message = "Failed to open the file",
        });
    }

    if (endline ? file.println(content) : file.print(content))
    {
        return ok();
    }

    return failure({
        .context = "WriteInFile",
        .message = "Failed to write in the file",
    });
}

/**
 * @brief Read a file
 *
 * @param path The path of the file
 * @param end The end of the line
 *
 * @return ErrorOr<utility::StringArray> can be the lines of the file or failure().
 */
auto ReadFromFile(String path, char end) -> ErrorOr<utility::StringArray>
{
    INTERNAL_DEBUG() << "ReadFromFile: " << path;

    if (!LittleFS.exists(path))
        return failure({
            .context = "ReadFromFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        return failure({
            .context = "ReadFromFile",
            .message = "Failed to open the file",
        });
    }

    utility::StringArray lines;
    String buff = "";

    while (file.available() > 0)
    {
        char c = (char)file.read();

        if (c == end)
        {
            lines.add(buff);
            buff = "";
            continue;
        }
        buff += c;
    }

    INTERNAL_DEBUG() << "ReadFromFile: " << lines.length() << " lines";

    return ok(lines);
}

/**
 * @brief Delete a file
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto DeleteFile(String path) -> ErrorOr<>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "DeleteFile",
            .message = "File does not exist",
        });

    if (LittleFS.remove(path))
        return ok();

    return failure({
        .context = "DeleteFile",
        .message = "Failed to delete the file",
    });
}

/**
 * @brief Clean a file
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto CleanFile(String path) -> ErrorOr<>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "CleanFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, "w");

    INTERNAL_DEBUG() << "Cleaning file '" << path << "'...";

    if (!file)
    {
        return failure({
            .context = "CleanFile",
            .message = "Failed to open the file",
        });
    }

    file.close();

    return ok();
}

/**
 * @brief Create a file
 *
 * @param path The path of the file
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto CreateFile(String path) -> ErrorOr<>
{
    if (LittleFS.exists(path))
        return failure({
            .context = "CreateFile",
            .message = "File already exists",
        });

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        return failure({
            .context = "CreateFile",
            .message = "Failed to create the file",
        });
    }

    file.close();

    return ok();
}

/**
 * @brief Check if a file exists
 *
 * @param path The path of the file
 *
 * @return bool true if the file exists, false otherwise
 */
auto FileExists(String path) -> bool
{
    return LittleFS.exists(path);
}

/**
 * @brief Open a file
 *
 * @param path The path of the file
 * @param mode The mode of the file
 *
 * @return ErrorOr<File> can be the file or failure()
 */
auto OpenFile(String path, String mode) -> ErrorOr<File>
{
    if (!LittleFS.exists(path))
        return failure({
            .context = "OpenFile",
            .message = "File does not exist",
        });

    File file = LittleFS.open(path, mode.c_str());

    if (!file)
    {
        return failure({
            .context = "OpenFile",
            .message = "Failed to open the file",
        });
    }

    return ok(file);
}

/**
 * @brief Close a file
 *
 * @param file The file to close
 *
 * @return ErrorOr<> can be ok() or failure()
 */
auto CloseFile(File &file) -> ErrorOr<>
{
    if (!file)
    {
        return failure({
            .context = "CloseFile",
            .message = "File is not open",
        });
    }

    INTERNAL_DEBUG() << "Closing file...";

    file.close();

    return ok();
}

/**
 * @brief Check if a file is empty
 *
 * @param path The path of the file
 *
 * @return bool true if the file is empty, false otherwise
 */
auto IsEmptyFile(String path) -> bool
{
    auto file = LittleFS.open(path, "r");
    bool empty = true;

    if (file)
    {
        empty = file.size() == 0;
        file.close();
    }

    return empty;
}

#endif // ! _File_h_