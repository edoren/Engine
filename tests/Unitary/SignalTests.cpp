#include "catch.hpp"

#include <cstdlib>
#include <sstream>

#include <System/Signal.hpp>
#include <System/String.hpp>

using namespace engine;

static std::stringstream gStream;

static const String gBaseMemberFunctionStr = "[Base Member Function]";
static const String gDerivedMemberFunctionStr = "[Derived Member Function]";
static const String gMemberFunctionStr = "[Member Function]";
static const String gStaticMemberFunctionStr = "[Static Member Function]";
static const String gStaticFunctionStr = "[Static Function]";
static const String gLambdaFunctionStr = "[Lambda Function]";

struct MyBase {
    virtual void VirtualMember() {
        gStream << gBaseMemberFunctionStr;
    }
};

struct MyClass : public MyBase {
    void VirtualMember() override {
        gStream << gDerivedMemberFunctionStr;
    }

    void Member() {
        gStream << gMemberFunctionStr;
    }

    static void StaticMember() {
        gStream << gStaticMemberFunctionStr;
    }
};

static void GlobalFunction() {
    gStream << gStaticFunctionStr;
}

static void MutipleArgs(int a, float b, double c, bool d) {
    gStream << std::boolalpha;
    gStream << a << ' ' << b << ' ' << c << ' ' << d;
}

auto LambdaFunction = []() { gStream << gLambdaFunctionStr; };

TEST_CASE("Signal creation", "[Signal]") {
    gStream.str(std::string());

    SECTION("Default constructor") {
        Signal<int> sig1;
        Signal<int, float> sig2;
        Signal<int, float, double> sig3;
        Signal<int, float, double, bool> sig4;
        sig1.emit(1);
        sig2.emit(1, 5.5f);
        sig3.emit(1, 5.5f, 0.1);
        sig4.emit(1, 5.5f, 0.1, false);
        REQUIRE(gStream.str() == std::string());
    }
    SECTION("Copy constructor") {
        const String result = gStaticFunctionStr + gStaticFunctionStr;
        Signal<> sig1;
        sig1.connect(GlobalFunction);
        Signal<> sig2(sig1);
        sig1.emit();
        sig2.emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Move constructor") {
        const String result = gStaticFunctionStr;
        Signal<> sig1;
        sig1.connect(GlobalFunction);
        Signal<> sig2(std::move(sig1));
        sig1.emit();
        sig2.emit();
        REQUIRE(gStream.str() == result);
    }
}

TEST_CASE("Signal connection", "[Signal]") {
    gStream.str(std::string());

    SECTION("Connect to a global function") {
        const String result = gStaticFunctionStr;
        Signal<> sig;
        sig.connect(GlobalFunction);
        sig.emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to a lambda") {
        const String result = gLambdaFunctionStr;
        Signal<> sig;
        sig.connect(LambdaFunction);
        sig.emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to a class non-static member function") {
        const String result = gMemberFunctionStr;
        MyClass instance;
        Signal<> sig;
        sig.connect(instance, &MyClass::Member);
        sig.emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to a class static member function") {
        const String result = gStaticMemberFunctionStr;
        Signal<> sig;
        sig.connect(MyClass::StaticMember);
        sig.emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to other signal") {
        const String result = gStaticFunctionStr;
        Signal<> sig1;
        Signal<> sig2;
        sig1.connect(GlobalFunction);
        sig2.connect(sig1, &Signal<>::emit);
        sig2.emit();
        REQUIRE(gStream.str() == result);
    }
}

TEST_CASE("Signal emmition", "[Signal]") {
    gStream.str(std::string());
    SECTION("Connect to a emit multiple values") {
        const String result = "1 5.5 0.1 false";
        Signal<int, float, double, bool> sig;
        sig.connect(MutipleArgs);
        sig.emit(1, 5.5f, 0.1, false);
        REQUIRE(gStream.str() == result);
    }
}
