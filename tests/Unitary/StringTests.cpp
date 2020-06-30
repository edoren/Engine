#include <catch2/catch.hpp>

#include <System/String.hpp>

using namespace engine;

TEST_CASE("String from other encodings", "[String]") {
    SECTION("from Wide strings") {
        const wchar* elements = L"水、火、地、風、空";
        String a = String(elements);
        String b = String(std::wstring(elements));
        String c = String::FromWide(elements, elements + wcslen(elements));
        REQUIRE(a.getSize() == 9);
        REQUIRE(a == u8"水、火、地、風、空");
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
    String hello = "مرحبا بالعالم";  // "Hello World" in Arabic
    String smiley = u8"\U0001F60A";

    SECTION("to Wide strings") {
        std::wstring helloWide = hello.toWide();
        REQUIRE(helloWide == L"مرحبا بالعالم");
    }
    SECTION("to UTF-8 strings") {
        const std::string& helloUtf8 = hello.toUtf8();
        const std::string& smileyUtf8 = smiley.toUtf8();
        REQUIRE(helloUtf8 == u8"مرحبا بالعالم");
        REQUIRE(smileyUtf8 == u8"\U0001F60A");
    }
    SECTION("to UTF-16 strings") {
        std::u16string helloUtf16 = hello.toUtf16();
        std::u16string smileyUtf16 = smiley.toUtf16();
        REQUIRE(helloUtf16 == u"مرحبا بالعالم");
        REQUIRE(smileyUtf16 == u"\U0001F60A");
    }
    SECTION("to UTF-32 strings") {
        std::u32string helloUtf32 = hello.toUtf32();
        std::u32string smileyUtf32 = smiley.toUtf32();
        REQUIRE(helloUtf32 == U"مرحبا بالعالم");
        REQUIRE(smileyUtf32 == U"\U0001F60A");
    }
}

TEST_CASE("String::find", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、地、風、空";

    SECTION("must be able to find any UTF-8 string") {
        size_t location = elements.find("風", 0);
        REQUIRE(location == 6);
    }
    SECTION("if no start is specified it start from the beginning") {
        size_t location = elements.find("水");
        REQUIRE(location == 0);
    }
    SECTION("it can start to search from any position") {
        size_t location1 = elements.find("地", 2);
        size_t location2 = elements.find("地", 4);
        REQUIRE(location1 == 4);
        REQUIRE(location1 == location2);
    }
    SECTION("if the string is not found it returns String::sInvalidPos") {
        size_t location1 = elements.find("A", 0);
        size_t location2 = elements.find("火", 5);
        REQUIRE(location1 == String::sInvalidPos);
        REQUIRE(location1 == location2);
    }
}

TEST_CASE("String::findFirstOf", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、地、風、空";

    SECTION("must be able to find any of the specified UTF-8 codepoints") {
        size_t location = elements.findFirstOf("火ñ地", 0);
        REQUIRE(location == 2);
    }
    SECTION("if no start is specified it start from the beginning") {
        size_t location = elements.findFirstOf("地火、");
        REQUIRE(location == 1);
    }
    SECTION("it can start to search from any position") {
        size_t location1 = elements.findFirstOf("、地火", 2);
        size_t location2 = elements.findFirstOf("空、地火", 4);
        REQUIRE(location1 == 2);
        REQUIRE(location2 == 4);
    }
    SECTION(
        "if the any of the UTF-8 characters are not found "
        "it returns String::sInvalidPos") {
        size_t location1 = elements.findFirstOf("A", 0);
        size_t location2 = elements.findFirstOf("#空ñ、風");
        REQUIRE(location1 == String::sInvalidPos);
        REQUIRE(location2 == 1);
    }
}

TEST_CASE("String::findLastOf", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、地、風、空";

    SECTION("must be able to find any of the specified UTF-8 codepoints") {
        size_t location = elements.findLastOf("火、地", elements.getSize() - 1);
        REQUIRE(location == 7);
    }
    SECTION("if no start is specified it start from the end") {
        size_t location = elements.findLastOf("風地火");
        REQUIRE(location == 6);
    }
    SECTION("it can start to search from any position") {
        size_t location1 = elements.findLastOf("、地火", 2);
        size_t location2 = elements.findLastOf("空、火", 4);
        REQUIRE(location1 == 2);
        REQUIRE(location2 == 3);
    }
    SECTION(
        "if the any of the UTF-8 characters are not found "
        "it returns String::sInvalidPos") {
        size_t location1 = elements.findLastOf("A");
        size_t location2 = elements.findLastOf("#空ñ、風");
        REQUIRE(location1 == String::sInvalidPos);
        REQUIRE(location2 == 8);
    }
}

TEST_CASE("String::replace", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、地、風、空";

    SECTION("must be able to replace any UTF-8 string") {
        elements.replace("水", "Water");
        elements.replace("地", "Earth");
        elements.replace("空", "Void");
        REQUIRE(elements == "Water、火、Earth、風、Void");
    }
    SECTION("must replace all the ocurrences of the provided string") {
        elements.replace("、", ", ");
        REQUIRE(elements == "水, 火, 地, 風, 空");
    }
    SECTION("could replace any element given a range") {
        elements.replace(4, 1, "Earth");
        elements.replace(0, 1, "Water");
        elements.replace(16, 1, "Void");
        REQUIRE(elements == "Water、火、Earth、風、Void");
    }
    SECTION("could replace the whole string") {
        elements.replace(0, elements.getSize(), "مرحبا بالعالم");
        REQUIRE(elements == "مرحبا بالعالم");
    }
    SECTION("must replace any Unicode code point with another") {
        // Replace U+3001 (、) with U+1F603 (😃)
        elements.replace(0x3001, 0x1F603);
        REQUIRE(elements == "水😃火😃地😃風😃空");
        // Replace U+1F603 (😃) with U+2D (-)
        elements.replace(0x1F603, '-');
        REQUIRE(elements == "水-火-地-風-空");
        // Replace U+2D (-) with U+20 (Space)
        elements.replace('-', ' ');
        REQUIRE(elements == "水 火 地 風 空");
    }
}

TEST_CASE("String::begin", "[String]") {
    // "Water, Fire, Earth, Wind, Void"
    String elements = "水、火、";
    auto it0 = elements.begin();

    SECTION("must be able to get the first character of the UTF-8 string") {
        char32 codePoint0 = std::apply(utf::CodePointFromUTF<8, const char8*>, *it0);
        REQUIRE(codePoint0 == 0x6C34);
    }
    SECTION("must be able to increment the UTF-8 iterator") {
        auto it1 = it0++;
        auto it2 = it0;
        auto it3 = ++it0;
        auto it4 = it0 + 1;
        auto it5 = it0;
        char32 codePoint1 = std::apply(utf::CodePointFromUTF<8, const char8*>, *it1);
        char32 codePoint2 = std::apply(utf::CodePointFromUTF<8, const char8*>, *it2);
        char32 codePoint3 = std::apply(utf::CodePointFromUTF<8, const char8*>, *it3);
        char32 codePoint4 = std::invoke(utf::CodePointFromUTF<8, const char8*>, it4->begin, it4->end);
        char32 codePoint5 = std::invoke(utf::CodePointFromUTF<8, const char8*>, it5->begin, it5->end);
        REQUIRE(codePoint1 == 0x6C34);
        REQUIRE(codePoint2 == 0x3001);
        REQUIRE(codePoint3 == 0x706B);
        REQUIRE(codePoint4 == 0x3001);
        REQUIRE(codePoint5 == 0x706B);
    }
    SECTION("must be able to iterate correctly through the UTF-8 string") {
        size_t count = 0;
        for (const auto& utfIt : elements) {
            char32 codePoint = std::apply(utf::CodePointFromUTF<8, const char8*>, utfIt);
            if (count == 0) {
                REQUIRE(codePoint == 0x6C34);
            } else if (count == 1) {
                REQUIRE(codePoint == 0x3001);
            } else if (count == 2) {
                REQUIRE(codePoint == 0x706B);
            } else if (count == 3) {
                REQUIRE(codePoint == 0x3001);
            }
            count++;
        }
    }
}
