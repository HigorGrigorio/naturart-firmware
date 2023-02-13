#ifndef ESP8266_STRING_HELPER
#define ESP8266_STRING_HELPER

#include <WString.h>
#include <StringArray.h>
#include <HardwareSerial.h>
#include <ErrorOr.h>

namespace utility
{
    class StringHelper
    {
    public:
        static ErrorOr<StringArray> splitStringToArray(const String &toSplit, const String &delimiter)
        {
            StringArray array;
            int start = 0;
            int end = toSplit.indexOf(delimiter);
            int last;

            if (end == -1)
            {
                return failure({.context = "Invalid delimiter",
                                .message = "cannot be found the delimiter"});
            }

            while (end != -1)
            {
                auto val = toSplit.substring(start, end - start);
                array.add(val);
                Serial.print(val);
                start = end + delimiter.length();
                last = end;
                end = toSplit.indexOf(delimiter, start);
            }

            array.add(toSplit.substring(last + 1, toSplit.length() - 1));

            return ok<StringArray>(array);
        }
    };
}
#endif //! ESP8266_STRING_HELPER
