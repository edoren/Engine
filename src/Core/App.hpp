#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Vector2.hpp>
#include <System/String.hpp>
#include <System/Time.hpp>

namespace engine {

class Main;

class ENGINE_API App {
    friend class Main;

protected:
    App();
    virtual ~App();

    virtual bool Initialize() = 0;

    virtual void Update() = 0;

    virtual void Shutdown() = 0;

    virtual String GetName() = 0;

    virtual math::Vector2<int32> GetWindowSize() = 0;

    const Time& GetDeltaTime();

private:
    Time m_delta_time;
};

}  // namespace engine
