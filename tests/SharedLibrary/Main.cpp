#include "Interface.hpp"

#include <System/SharedLibrary.hpp>
#include <System/FileSystem.hpp>

#include <iostream>
#include <string>

using namespace engine;

int main(int /*argc*/, char* argv[]) {
    String basedir = filesystem::ExecutableDirectory();

    basedir = filesystem::Join(basedir, "Impl1");
    SharedLibrary library(basedir);
    library.Load();
    Operation* op = static_cast<Operation*>(library.GetSymbol("impl"));

    op->Operate(2, 5);

    std::cout << op->GetName() << std::endl;
    std::cout << 2 << " " << 5 << std::endl;
    std::cout << op->GetLastResult() << std::endl;
    return 0;
}
