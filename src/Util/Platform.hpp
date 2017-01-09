#pragma once

/*
    This header file defines some useful macros to detect the current
    running system, as well as which system APIs are avaliable on it.
*/

#define PLATFORM_WINDOWS 0x1
#define PLATFORM_LINUX 0x2
#define PLATFORM_MAC 0x4
#define PLATFORM_ANDROID 0x8
#define PLATFORM_IOS 0xF

#define PLATFORM_TYPE_DESKTOP 1
#define PLATFORM_TYPE_MOBILE 2

#define PLATFORM_API_WIN32 0x1
#define PLATFORM_API_WIN64 0x2
#define PLATFORM_API_POSIX 0x4

// Find the current platform and platform type
#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#define PLATFORM_TYPE PLATFORM_TYPE_DESKTOP
#elif defined(__linux__)
#if !defined(__ANDROID__)
#define PLATFORM PLATFORM_LINUX
#define PLATFORM_TYPE PLATFORM_TYPE_DESKTOP
#else
#define PLATFORM PLATFORM_ANDROID
#define PLATFORM_TYPE PLATFORM_TYPE_MOBILE
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
#define PLATFORM PLATFORM_IOS
#define PLATFORM_TYPE PLATFORM_TYPE_MOBILE
#elif TARGET_OS_MAC == 1
#define PLATFORM PLATFORM_MAC
#define PLATFORM_TYPE PLATFORM_TYPE_DESKTOP
#endif
#else
#define PLATFORM PLATFORM_UNKNOWN
#define PLATFORM_TYPE PLATFORM_TYPE_UNKNOWN
#endif

// Detect the platform avaliable APIs
#if defined(_WIN32)
#if defined(_WIN64)
#define PLATFORM_APIS PLATFORM_API_WIN32 & PLATFORM_API_WIN64
#else
#define PLATFORM_APIS PLATFORM_API_WIN32
#endif
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) /* UNIX-style OS. */
#if defined(_POSIX_VERSION) /* POSIX compliant */
#undef PLATFORM_APIS
#define PLATFORM_APIS PLATFORM_API_POSIX
#endif
#endif

#define PLATFORM_IS(x) PLATFORM == x
#define PLATFORM_TYPE_IS(x) PLATFORM_TYPE == x
#define PLATFORM_HAS_API(x) PLATFORM_APIS & x
