#pragma once

#include <Util/Prerequisites.hpp>
#include <System/String.hpp>

#define FMT_HEADER_ONLY
#define FMT_USE_USER_DEFINED_LITERALS 0
#include <fmt/format.h>

namespace engine {

template <typename Char>
struct UdlStringFormatProxy {
    const Char* str;

    template <typename... Args>
    String operator()(Args&&... args) const {
        return String(fmt::format(str, std::forward<Args>(args)...));
    }
};

inline UdlStringFormatProxy<char8> operator"" _format(const char8* str,
                                                      std::size_t) {
    return {str};
}

inline UdlStringFormatProxy<wchar> operator"" _format(const wchar* str,
                                                      std::size_t) {
    return {str};
}

}  // namespace engine
