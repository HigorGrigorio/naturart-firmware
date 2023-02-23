/**
 * @file Error.h
 * @brief This file contains the implementation of the Error struct.
 * @details This file contains the implementation of the Error struct
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 * 
*/

#ifndef _Error_h_
#define _Error_h_

#include <Print.h>
#include <Printable.h>
#include <WString.h>

struct Error
{
    const char *context = "";
    const char *message = "";

    static Error None;

    friend auto operator<<(internal::ExitingStream stream, Error e) -> internal::ExitingStream&;
};

auto operator<<(internal::ExitingStream stream, Error e) -> internal::ExitingStream&
{
    return stream << "Error{" << e.context << ":" << e.message << "}";
}

Error Error::None = {.context="<nocontext>", .message="<nomessage>"};

#endif // ! _Error_h_