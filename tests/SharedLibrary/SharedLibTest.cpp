#include "Operation.hpp"

#include <System/FileSystem.hpp>
#include <System/SharedLibrary.hpp>
#include <System/String.hpp>

#include <iostream>

typedef Operation* (*PFN_GetOperation)(void);

using namespace engine;

bool Operate(const String& lib_name, float a, float b) {
    SharedLibrary lib(lib_name);
    bool loaded = lib.Load();
    if (loaded) {
        PFN_GetOperation fn = (PFN_GetOperation)lib.GetSymbol("GetOperation");
        Operation* op = fn();
        std::cout << "Lib: " << lib.GetName().ToUtf8() << std::endl;
        std::cout << "Name: " << op->GetName() << std::endl;
        std::cout << "Result: " << op->Operate(a, b) << std::endl;
        lib.Unload();
        return true;
    } else {
        std::cout << "Error: " << lib.GetErrorString().ToUtf8() << std::endl;
        return false;
    }
}

int main(int /*argc*/, char* /*argv*/ []) {
    float a = 7;
    float b = 2;

    String lib1_name = "OperationAdd";
    String lib2_name = "OperationMult";

    std::cout << "======= Numbers =======" << std::endl;
    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "===== 1st Library =====" << std::endl;
    if (!Operate(lib1_name, a, b)) return 1;
    std::cout << "===== 2nd Library =====" << std::endl;
    if (!Operate(lib2_name, a, b)) return 1;
    std::cout << "=======================" << std::endl;
    return 0;
}
