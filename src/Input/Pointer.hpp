#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>

namespace engine {

struct ENGINE_API Pointer {
    SDL_FingerID id;
    math::ivec2 mousepos;
    math::ivec2 mousedelta;
    bool used;

    Pointer() : id(0), mousepos(-1), mousedelta(0), used(false) {}
};

}  // namespace engine
