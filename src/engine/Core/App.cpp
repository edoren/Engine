#include <Core/App.hpp>

namespace engine {

namespace {

const String sTag("App");

}  // namespace

App::App() : m_delta_time(Time::sZero) {}

App::~App() {}

const Time& App::getDeltaTime() {
    return m_delta_time;
}

}  // namespace engine
