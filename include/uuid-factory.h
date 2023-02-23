/**
 * @file uuid.h
 * @brief This file contains the implementation of the UUID helpers.
 * @details This file contains the implementation of the UUID helpers
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 *
 */

#ifndef _UUID_h
#define _UUID_h

#include <UUID.h>
#include <WString.h>

auto makeUUID() -> String {
    auto factory = UUID();

    factory.setVariant4Mode();
    factory.generate();

   return String(factory.toCharArray());
}

#endif // !_UUID_h