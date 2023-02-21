/**
 * @file internal.h
 * @author your name (you@domain.com)
 * @brief A internal stream to receives an runtime error.
 * @version 0.1
 * @date 2023-01-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _Internal_h_
#define _Internal_h_

#include <WString.h>
#include <HardwareSerial.h>

#include <type_traits>

namespace internal
{
    // Wraps a stream and exiting for FATAL errors. Should only be used by Check.h
    class ExitingStream
    {
    public:
        // A tag type that renders as ": " in an ExitingStream, but only if it is
        // followed by additional output. Otherwise, it renders as "". Primarily used
        // when building macros around these streams.
        struct AddSeparator
        {
        };

        // Internal type used in macros to dispatch to the `operator|` overload.
        struct Helper
        {
        };

        // Internal type used in macros to dispatch the message to the `operator<<` without stop aplication.
        struct Debug
        {
        };

        ExitingStream()
            : buffer_() {}

        // Never called.
        ~ExitingStream()
        {
            buffer_.concat(
                "Exiting streams should only be constructed by CHECK.h macros that "
                "ensure the special operator| exits the program prior to their "
                "destruction!");
        }

        // If the bool cast occurs, it's because the condition is false. This supports
        // && short-circuiting the creation of ExitingStream.
        explicit operator bool() const { return true; }

        // Overloads for all the types that can be printed to a stream. This is
        // necessary to support the Check.h macros.
        template <typename T, typename std::enable_if_t<
                                  std::disjunction_v<
                                      std::is_same<T, char *>,
                                      std::is_same<T, const char *>,
                                      std::is_same<T, String>,
                                      std::is_same<T, char>,
                                      std::is_same<T, int>,
                                      std::is_same<T, long>,
                                      std::is_same<T, unsigned int>,
                                      std::is_same<T, unsigned long>,
                                      std::is_same<T, float>,
                                      std::is_same<T, double>,
                                      std::is_same<T, bool>>> * = nullptr>
        auto operator<<(T message)
            -> ExitingStream &
        {
            if (separator_)
            {
                buffer_.concat(": ");
                separator_ = false;
            }
            buffer_.concat(message);
            return *this;
        }

        // Marks the stream to print a separator if it is used again.
        auto operator<<(AddSeparator /*add_separator*/) -> ExitingStream &
        {
            separator_ = true;
            return *this;
        }

        // Low-precedence binary operator overload used in CHECK.h macros to flush the
        // debug output.
        friend auto operator|(Debug /*debug*/, ExitingStream &stream)
            -> void
        {
            stream.debug();
        }

        // Low-precedence binary operator overload used in CHECK.h macros to flush the
        // output and exit the program. We do this in a binary operator rather than
        // the destructor to ensure good debug info and backtraces for errors.
        friend auto operator|(Helper /*helper*/, ExitingStream &stream)
            -> void
        {
            stream.done();
        }

    private:
        auto done() -> void
        {
            buffer_.concat("\n");

            // Print's the error message.
            Serial.print(buffer_);

            // It's useful to exit the program with `std::abort()` for integration with
            // debuggers and other tools.
            std::abort();
        }

        auto debug() -> void
        {
            buffer_.concat("\n");

            // Print's the error message.
            Serial.print(buffer_);

            buffer_ = "";
        }

        // Whether a separator should be printed if << is used again.
        bool separator_ = false;

        String buffer_;
    };
}

// Raw exiting stream. This should be used when building forms of exiting
// macros. It evaluates to a temporary `ExitingStream` object that can be
// manipulated, streamed into, and then will exit the program.

#define check_internal_stream() \
    internal::ExitingStream::Helper() | internal::ExitingStream()

#define check_internal_debug() \
    internal::ExitingStream::Debug() | internal::ExitingStream()

#endif // ! _Internal_h_