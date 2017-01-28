#include <Renderer/RenderWindow.hpp>

namespace engine {

RenderWindow::RenderWindow()
      : name_(""),
        size_(0, 0),
        is_fullscreen_(false),
        is_vsync_enable_(false) {}

RenderWindow::~RenderWindow() {}

}  // namespace engine
