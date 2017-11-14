#pragma once

#include <Math/Math.hpp>
#include <Util/Prerequisites.hpp>

namespace engine {

struct UniformBufferObject {
    math::Matrix4x4<float> model;
    math::Matrix4x4<float> normalMatrix;
    math::Matrix4x4<float> mvp;
    math::Vector3Packed<float> cameraFront;
    math::Vector3Packed<float> lightPosition;
};

}  // namespace engine
