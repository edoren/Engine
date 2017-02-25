#include "catch.hpp"

#include "OStreamOverloads.hpp"

#include <System/FileSystem.hpp>
#include <System/String.hpp>

using namespace engine;

TEST_CASE("filesystem::IsAbsolute", "[FileSystem]") {
    SECTION("true if the path is absolute, false otherwise") {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        String absolute = "C:\\hello";
        String relative = "\\hello\\world";
#else
        String absolute = "/hello";
        String relative = "hello/world";
#endif
        REQUIRE(filesystem::IsAbsolute(absolute) == true);
        REQUIRE(filesystem::IsAbsolute(relative) == false);
    }
}

TEST_CASE("filesystem::NormalizePath", "[FileSystem]") {
    SECTION("must resolve all the .. directories") {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        String path1 = "hello\\world\\..";
        String path2 = "hello\\..\\world\\..";
        String path3 = "hello\\..\\world\\..\\..";
        String path4 = "hello\\..\\world\\..\\..\\..";
#else
        String path1 = "hello/world/..";
        String path2 = "hello/../world/..";
        String path3 = "hello/../world/../..";
        String path4 = "hello/../world/../../..";
#endif
        String path1_norm = filesystem::NormalizePath(path1);
        String path2_norm = filesystem::NormalizePath(path2);
        String path3_norm = filesystem::NormalizePath(path3);
        String path4_norm = filesystem::NormalizePath(path4);
        REQUIRE(path1_norm == "hello");
        REQUIRE(path2_norm == ".");
        REQUIRE(path3_norm == "..");
        REQUIRE(path4_norm == "../..");
    }
    SECTION("must resolve all the . directories") {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        String path = "hello\\.\\world\\.\\.\\.";
#else
        String path = "hello/./world/././.";
#endif
        String path_norm = filesystem::NormalizePath(path);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        REQUIRE(path_norm == "hello\\world");
#else
        REQUIRE(path_norm == "hello/world");
#endif
    }
    SECTION("must remove all the leftover separators") {
#if PLATFORM_IS(PLATFORM_WINDOWS)
        String path1 = "hello\\\\1234\\\\world\\\\.\\\\\\";
#else
        String path1 = "hello//1234//world//.///";
#endif
        String path1_norm = filesystem::NormalizePath(path1);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        REQUIRE(path1_norm == "hello\\1234\\world");
#else
        REQUIRE(path1_norm == "hello/1234/world");
#endif
    }
    SECTION("if absolute must keep the root component") {
#if PLATFORM_IS(PLATFORMWINDOWS)
        String path1 = "C:\\hello";
        String path2 = "C:\\hello\\..\\..";
        String path3 = "C:\\..\\..";
#else
        String path1 = "/hello";
        String path2 = "/hello/../..";
        String path3 = "/../..";
#endif
        String path1_norm = filesystem::NormalizePath(path1);
        String path2_norm = filesystem::NormalizePath(path2);
        String path3_norm = filesystem::NormalizePath(path3);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        REQUIRE(path1_norm == "C:\\hello");
        REQUIRE(path2_norm == "C:\\");
        REQUIRE(path3_norm == "C:\\");
#else
        REQUIRE(path1_norm == "/hello");
        REQUIRE(path2_norm == "/");
        REQUIRE(path3_norm == "/");
#endif
    }
}

TEST_CASE("filesystem::Join", "[FileSystem]") {
    SECTION("must be able to join any number of paths") {
        String joined = filesystem::Join("hello", "world", "1234");
#if PLATFORM_IS(PLATFORM_WINDOWS)
        REQUIRE(joined == "hello\\world\\1234");
#else
        REQUIRE(joined == "hello/world/1234");
#endif
    }
    SECTION("if any of the provided paths is empty it must ignore it") {
        String joined = filesystem::Join("hello", "", "1234");
#if PLATFORM_IS(PLATFORM_WINDOWS)
        REQUIRE(joined == "hello\\1234");
#else
        REQUIRE(joined == "hello/1234");
#endif
    }
    SECTION("if the right element is an absolute path it must return it") {
        String left = "hello";
#if PLATFORM_IS(PLATFORM_WINDOWS)
        String right = "C:\\world\\1234";
#else
        String right = "/world/1234";
#endif
        String joined = filesystem::Join(left, right);
#if PLATFORM_IS(PLATFORM_WINDOWS)
        REQUIRE(joined == "C:\\world\\1234");
#else
        REQUIRE(joined == "/world/1234");
#endif
    }
    SECTION(
        "must add a separator if the left argument does not finish in one") {
        String left1 = "hello";
#if PLATFORM_IS(PLATFORM_WINDOWS)
        String left2 = "hello\\";
#else
        String left2 = "hello/";
#endif
        String right = "world";
        String joined1 = filesystem::Join(left1, right);
        String joined2 = filesystem::Join(left2, right);
        REQUIRE(joined1 == joined2);
    }
}
