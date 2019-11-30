#pragma once

namespace engine {

namespace math {

template <typename T>
constexpr const T& Clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

template <typename T0, typename T1>
typename std::common_type<T0, T1>::type Max(const T0& val1, const T1& val2) {
    return val1 > val2 ? val1 : val2;
}

template <typename T0, typename T1, typename... Args>
typename std::common_type<T0, T1, Args...>::type Max(const T0& val1, const T1& val2, const Args&... args) {
    return Max(Max(val1, val2), std::forward<Args>(args)...);
}

template <typename T0, typename T1>
typename std::common_type<T0, T1>::type Min(const T0& val1, const T1& val2) {
    return val1 < val2 ? val1 : val2;
}

template <typename T0, typename T1, typename... Args>
typename std::common_type<T0, T1, Args...>::type Min(const T0& val1, const T1& val2, const Args&... args) {
    return Min(Min(val1, val2), std::forward<Args>(args)...);
}

}  // namespace math

}  // namespace engine
