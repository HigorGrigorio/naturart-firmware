/**
 * @file UtilStringArray.h
 * @brief This file contains the implementation of the StringArray class.
 * @details This file contains the implementation of the StringArray class
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 * 
*/
#ifndef ESP8266_STRING_ARRAY
#define ESP8266_STRING_ARRAY

#include <WString.h>
#include <LinkedList.h>

namespace utility
{
    class StringArray : public LinkedList<String>
    {
    public:
        StringArray() : LinkedList() {}

        bool containsIgnoreCase(const String &str)
        {
            for (const auto &s : *this)
            {
                if (str.equalsIgnoreCase(s))
                {
                    return true;
                }
            }
            return false;
        }
    };
}

#endif // !ESP8266_STRING_ARRAY
