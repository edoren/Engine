#include <Core/App.hpp>

namespace engine {

namespace {

const String sTag("App");

}  // namespace

App::App() : m_deltaTime(Time::sZero) {}

App::~App() {}

const Time& App::getDeltaTime() {
    return m_deltaTime;
}

}  // namespace engine
