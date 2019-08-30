#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class RenderWindow;
class RenderStates;

class ENGINE_API Drawable {
public:
    ////////////////////////////////////////////////////////////
    /// @brief Virtual destructor
    ////////////////////////////////////////////////////////////
    virtual ~Drawable() {}

protected:
    friend class RenderWindow;

    ////////////////////////////////////////////////////////////
    /// @brief Draw the object to a RenderWindow
    ///
    /// This is a pure virtual function that has to be implemented
    /// by the derived class to define how the drawable should be
    /// drawn.
    ///
    /// @param target RenderWindow to draw to
    ////////////////////////////////////////////////////////////
    virtual void Draw(RenderWindow& target,
                      const RenderStates& states) const = 0;
};

}  // namespace engine
