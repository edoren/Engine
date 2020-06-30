#include <Core/App.hpp>

#include <System/StringView.hpp>

namespace engine {

namespace {

// const StringView sTag("App");

}  // namespace

App::App() : m_deltaTime(Time::sZero) {}

App::~App() = default;

const Time& App::getDeltaTime() {
    return m_deltaTime;
}

}  // namespace engine
