#pragma once

#include <Util/Prerequisites.hpp>

#include <type_traits>

namespace engine {

namespace type {

template <typename T>
using iterator_underlying_type = std::decay<decltype(*(std::declval<T>()))>;

template <typename T>
using iterator_underlying_type_t = typename iterator_underlying_type<T>::type;

template <typename T>
struct ENGINE_API is_bidirectional_iterator
      : public std::integral_constant<bool,
                                      (std::is_arithmetic<iterator_underlying_type_t<T>>::value ||
                                       std::is_compound<iterator_underlying_type_t<T>>::value) &&
                                          std::is_same<decltype(++(std::declval<T&>())), T&>::value &&
                                          std::is_same<decltype(--(std::declval<T&>())), T&>::value> {};

template <typename T>
struct ENGINE_API is_forward_iterator
      : public std::integral_constant<bool,
                                      (std::is_arithmetic<iterator_underlying_type_t<T>>::value ||
                                       std::is_compound<iterator_underlying_type_t<T>>::value) &&
                                          std::is_same<decltype(++(std::declval<T&>())), T&>::value> {};

template <typename T>
struct ENGINE_API alignment_of : std::integral_constant<size_t, alignof(T)> {};

template <typename T>
struct ENGINE_API size_of : std::integral_constant<size_t, sizeof(T)> {};

template <typename T>
using alignment_of_v = typename alignment_of<T>::value;

template <typename T>
using size_of_v = typename size_of<T>::value;

}  // namespace type

}  // namespace engine
