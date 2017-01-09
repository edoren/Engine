#include "Interface.hpp"

#include <System/DLL/SharedLibrary.hpp>
#include <System/FileSystem.hpp>

#include <iostream>
#include <string>

using namespace engine;

int main(int argc, char* argv[]) {
    filesystem::Path basedir = filesystem::Absolute(argv[0]).ParentPath();

    SharedLibrary library(basedir / "Impl2");
    Operation* op = library.LoadSymbol<Operation>("impl");

    op->Operate(2, 5);

    std::cout << op->GetName() << std::endl;
    std::cout << 2 << " " << 5 << std::endl;
    std::cout << op->GetLastResult() << std::endl;
    return 0;
}
