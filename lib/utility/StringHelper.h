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
        static ErrorOr<StringArray> splitStringToArray(const String &toSplit, const char delimiter)
        {
            StringArray array;
            String buff;

            for (char c : toSplit)
            {
                if (c != delimiter)
                {
                    buff.concat(c);
                }
                else
                {
                    INTERNAL_DEBUG() << "Splitting the payload: " << buff << " (delimiter: " << delimiter << ")";
                    array.add(buff);
                    buff.clear();
                }
            }

            INTERNAL_DEBUG() << "Splitting the payload: " << buff << " (delimiter: " << delimiter << ")";
            array.add(buff);

            return ok<StringArray>(array);
        }
    };
}
#endif //! ESP8266_STRING_HELPER
