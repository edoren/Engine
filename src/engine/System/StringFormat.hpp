#pragma once

#include <System/String.hpp>
#include <Util/Prerequisites.hpp>

#define FMT_HEADER_ONLY
#define FMT_USE_WINDOWS_H 0
#define FMT_USE_USER_DEFINED_LITERALS 0
#ifndef ENGINE_EXPORTS
    #define FMT_SHARED 0
#endif
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace engine {

template <typename Char>
struct UdlStringFormatProxy {
    const Char* str;

    template <typename... Args>
    String operator()(Args&&... args) const {
        return String(fmt::format(str, std::forward<Args>(args)...));
    }
};

inline UdlStringFormatProxy<char8> operator"" _format(const char8* str, std::size_t) {
    return {str};
}

inline UdlStringFormatProxy<wchar> operator"" _format(const wchar* str, std::size_t) {
    return {str};
}

}  // namespace engine

FMT_BEGIN_NAMESPACE

template <typename T>
struct formatter<std::vector<T>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext = format_context>
    auto format(const std::vector<T>& v, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "[{}]", fmt::join(v, ", "));
    }
};

FMT_END_NAMESPACE
