#include "catch.hpp"
#include "OStreamOverloads.hpp"

#include <System/String.hpp>

using namespace engine;

TEST_CASE("String from other encodings", "[String]") {
    SECTION("from Wide strings") {
        const wchar* hello = L"HELLO WORLD";
        String a = String(hello);
        String b = String(std::wstring(hello));
        String c = String::FromWide(hello, hello + wcslen(hello));
        REQUIRE(a == u8"HELLO WORLD");
        REQUIRE(a == b);
        REQUIRE(b == c);
    }
    SECTION("from UTF-8 strings") {
        const char8* smiley = u8"\U0001F601";
        String a = String(smiley);
        String b = String(std::string(smiley));
        String c = String::FromUtf8(smiley, smiley + 4);
        REQUIRE(a == u8"\U0001F601");
        REQUIRE(a == b);
        REQUIRE(b == c);
    }
    SECTION("from UTF-16 strings") {
        const char16* smiley = u"\U0001F601";
        String a = String(smiley);
        String b = String(std::u16string(smiley));
        String c = String::FromUtf16(smiley, smiley + 2);
        REQUIRE(a == u8"\U0001F601");
        REQUIRE(a == b);
        REQUIRE(b == c);
    }
    SECTION("from UTF-32 strings") {
        const char32* smiley = U"\U0001F601";
        String a = String(smiley);
        String b = String(std::u32string(smiley));
        String c = String::FromUtf32(smiley, smiley + 1);
        REQUIRE(a == u8"\U0001F601");
        REQUIRE(a == b);
        REQUIRE(b == c);
    }
}

TEST_CASE("String to other encodings", "[String]") {
    String hello = "HELLO WORLD";
    String smiley = u8"\U0001F60A";

    SECTION("to Wide strings") {
        std::wstring hello_wide = hello.ToWide();
        REQUIRE(hello_wide == L"HELLO WORLD");
    }
    SECTION("to UTF-8 strings") {
        std::string hello_utf8 = hello.ToUtf8();
        std::string smiley_utf8 = smiley.ToUtf8();
        REQUIRE(hello_utf8 == u8"HELLO WORLD");
        REQUIRE(smiley_utf8 == u8"\U0001F60A");
    }
    SECTION("to UTF-16 strings") {
        std::u16string hello_utf16 = hello.ToUtf16();
        std::u16string smiley_utf16 = smiley.ToUtf16();
        REQUIRE(hello_utf16 == u"HELLO WORLD");
        REQUIRE(smiley_utf16 == u"\U0001F60A");
    }
    SECTION("to UTF-32 strings") {
        std::u32string hello_utf32 = hello.ToUtf32();
        std::u32string smiley_utf32 = smiley.ToUtf32();
        REQUIRE(hello_utf32 == U"HELLO WORLD");
        REQUIRE(smiley_utf32 == U"\U0001F60A");
    }
}

TEST_CASE("String::Find", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、地、風、空";

    SECTION("must be able to find any UTF-8 string") {
        size_t location = elements.Find("風");
        REQUIRE(location == 6);
    }
    SECTION("if no start is specified it start from the beginning") {
        size_t location = elements.Find("水");
        REQUIRE(location == 0);
    }
    SECTION("it can start to search from any position") {
        size_t location1 = elements.Find("地", 2);
        size_t location2 = elements.Find("地", 4);
        REQUIRE(location1 == 4);
        REQUIRE(location1 == location2);
    }
    SECTION("if the string is not found it returns String::InvalidPos") {
        size_t location1 = elements.Find("A");
        size_t location2 = elements.Find("火", 5);
        REQUIRE(location1 == String::InvalidPos);
        REQUIRE(location1 == location2);
    }
}

TEST_CASE("String::Replace", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、地、風、空";

    SECTION("must be able to replace any UTF-8 string") {
        elements.Replace("水", "Water");
        elements.Replace("地", "Earth");
        elements.Replace("空", "Void");
        REQUIRE(elements == "Water、火、Earth、風、Void");
    }
    SECTION("must replace all the ocurrences of the provided string") {
        elements.Replace("、", ", ");
        REQUIRE(elements == "水, 火, 地, 風, 空");
    }
    SECTION("could replace any element given a range") {
        elements.Replace(4, 1, "Earth");
        elements.Replace(0, 1, "Water");
        elements.Replace(16, 1, "Void");
        REQUIRE(elements == "Water、火、Earth、風、Void");
    }
    SECTION("could replace the whole string") {
        elements.Replace(0, elements.GetSize(), "Hello World");
        REQUIRE(elements == "Hello World");
    }
}
