#pragma once

#include <Util/Precompiled.hpp>

namespace engine {

struct Pointer {
    SDL_FingerID id;
    math::ivec2 mousepos;
    math::ivec2 mousedelta;
    bool used;

    Pointer() : id(0), mousepos(-1), mousedelta(0), used(false) {}
};

}  // namespace engine
