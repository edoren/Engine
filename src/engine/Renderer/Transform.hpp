#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Matrix4x4.hpp>
#include <Math/Vector3.hpp>

namespace engine {

class ENGINE_API Transform {
public:
    Transform();
    Transform(const Transform& other);
    Transform(Transform&& other);
    ~Transform();

    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other);

    void Rotate(const math::Vector3<float>& euler_angles);
    void Scale(const math::Vector3<float>& scale);
    void Translate(const math::Vector3<float>& translate);

    math::Matrix4x4<float> GetMatrix() const;

private:
    math::Vector3<float> m_scale;
    math::Vector3<float> m_rotate;
    math::Vector3<float> m_translate;
};

}  // namespace engine
