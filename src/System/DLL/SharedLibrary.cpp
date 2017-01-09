#include <System/DLL/SharedLibrary.hpp>

namespace engine {

SharedLibrary::SharedLibrary(const filesystem::Path& path)
      : impl_(new priv::SharedLibraryImpl(path)) {}

SharedLibrary::~SharedLibrary() {
    delete impl_;
}

}  // namespace engine
