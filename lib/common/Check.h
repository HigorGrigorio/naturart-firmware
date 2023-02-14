/**
 * @file check.h
 * @author Higor Grigorio.
 * @brief Helpers to perform exception checking at runtime.
 * @version 0.1
 * @date 2023-01-25
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _Check_h
#define _Check_h

#include <Internal.h>

// Checks the given condition, and if it's false, streams the
// error message, then exits. This should be used for unexpected errors, such as
// a bug in the application.
//
// For example:
//   CHECK(is_valid) << "Data is not valid!";
#define CHECK(...) (__VA_ARGS__) ? (void)0                                                   \
                                 : check_internal_stream()                                   \
                                       << "CHECK failure at " << __FILE__ << ":" << __LINE__ \
                                       << ": " #__VA_ARGS__                                  \
                                       << internal::ExitingStream::AddSeparator()

// This is similar to CHECK, but is unconditional. Writing FATAL() is
// clearer than CHECK(false) because it avoids confusion about control
// flow.
//
// For example:
//   FATAL() << "Unreachable!";
#define FATAL()             \
    check_internal_stream() \
        << "FATAL failure at " << __FILE__ << ":" << __LINE__ << ": "

// Unlike CHECK, this does not abort the application,
// it only displays the debug message, making it easier
// to trace the application.
//
// For example:
//   INTERNAL_DEBUG() << "Safe block!";
#define INTERNAL_DEBUG() check_internal_debug()                                                  \
                             << "DEBUG at " << __FILE__ << ":" << __LINE__ \
                             << internal::ExitingStream::AddSeparator()

#endif // ! _Check_h
