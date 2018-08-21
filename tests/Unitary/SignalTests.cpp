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
        sig1.Emit(1);
        sig2.Emit(1, 5.5f);
        sig3.Emit(1, 5.5f, 0.1);
        sig4.Emit(1, 5.5f, 0.1, false);
        REQUIRE(gStream.str() == std::string());
    }
    SECTION("Copy constructor") {
        const String result = gStaticFunctionStr + gStaticFunctionStr;
        Signal<> sig1;
        sig1.Connect(GlobalFunction);
        Signal<> sig2(sig1);
        sig1.Emit();
        sig2.Emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Move constructor") {
        const String result = gStaticFunctionStr;
        Signal<> sig1;
        sig1.Connect(GlobalFunction);
        Signal<> sig2(std::move(sig1));
        sig1.Emit();
        sig2.Emit();
        REQUIRE(gStream.str() == result);
    }
}

TEST_CASE("Signal connection", "[Signal]") {
    gStream.str(std::string());

    SECTION("Connect to a global function") {
        const String result = gStaticFunctionStr;
        Signal<> sig;
        sig.Connect(GlobalFunction);
        sig.Emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to a lambda") {
        const String result = gLambdaFunctionStr;
        Signal<> sig;
        sig.Connect(LambdaFunction);
        sig.Emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to a class non-static member function") {
        const String result = gMemberFunctionStr;
        MyClass instance;
        Signal<> sig;
        sig.Connect(instance, &MyClass::Member);
        sig.Emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to a class static member function") {
        const String result = gStaticMemberFunctionStr;
        Signal<> sig;
        sig.Connect(MyClass::StaticMember);
        sig.Emit();
        REQUIRE(gStream.str() == result);
    }
    SECTION("Connect to other signal") {
        const String result = gStaticFunctionStr;
        Signal<> sig1;
        Signal<> sig2;
        sig1.Connect(GlobalFunction);
        sig2.Connect(sig1, &Signal<>::Emit);
        sig2.Emit();
        REQUIRE(gStream.str() == result);
    }
}

TEST_CASE("Signal emmition", "[Signal]") {
    gStream.str(std::string());
    SECTION("Connect to a emit multiple values") {
        const String result = "1 5.5 0.1 false";
        Signal<int, float, double, bool> sig;
        sig.Connect(MutipleArgs);
        sig.Emit(1, 5.5f, 0.1, false);
        REQUIRE(gStream.str() == result);
    }
}
