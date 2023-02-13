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