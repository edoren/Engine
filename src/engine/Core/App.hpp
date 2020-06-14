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

    ////////////////////////////////////////////////////////////
    /// @brief Initialize the application
    ///
    /// @remark This is called before creating the RenderWindow
    ////////////////////////////////////////////////////////////
    virtual bool initialize() = 0;

    ////////////////////////////////////////////////////////////
    /// @brief Update the application the application
    ///
    /// @remark This is called each frame
    ///
    /// @see GetDeltaTime
    ////////////////////////////////////////////////////////////
    virtual void update() = 0;

    ////////////////////////////////////////////////////////////
    /// @brief Shutdown the application
    ///
    /// @remarks Called when the engine is closing, you should cleanup
    ///          any allocated resources in this method.
    ///
    /// @see Initialize
    ////////////////////////////////////////////////////////////
    virtual void shutdown() = 0;

    ////////////////////////////////////////////////////////////
    /// @brief The obtain the name of the application
    ///
    /// @return String containing the name
    ////////////////////////////////////////////////////////////
    virtual String getName() = 0;

    ////////////////////////////////////////////////////////////
    /// @brief The intitial window size of the application
    ///
    /// @return 2D Vector containing the window size
    ////////////////////////////////////////////////////////////
    virtual math::Vector2<int32> getWindowSize() = 0;

    ////////////////////////////////////////////////////////////
    /// @brief Obtains the delta time of the last frame
    ///
    /// @remarks You can use this method from the Update method
    ///          to get the delta time of the last frame
    ///
    /// @return A reference to a Time object containig the delta
    ///         time of the last frame
    ////////////////////////////////////////////////////////////
    const Time& getDeltaTime();

private:
    Time m_deltaTime;
};

}  // namespace engine
