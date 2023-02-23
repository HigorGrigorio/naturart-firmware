/**
 * @file Nonnull.h
 * @brief This file contains the implementation of the Nonnull type.
 * @details This file contains the implementation of the Nonnull type
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 * 
*/
#ifndef _Nonnull_h_
#define _Nonnull_h_

#include <type_traits>

// A non-nullable pointer. Written as `Nonnull<T*>` instead of `T*`.
//
// Sanitizers enforce this dynamically on assignment, return, and when passing
// as an argument. Static analysis will also track erroneous uses of `nullptr`.
template <typename T,
          typename std::enable_if_t<std::is_pointer_v<T>> * = nullptr>
using Nonnull = T _Nonnull;

#endif // _Nonnull_h_