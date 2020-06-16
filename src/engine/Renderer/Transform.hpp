#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Matrix4x4.hpp>
#include <Math/Vector3.hpp>

namespace engine {

class ENGINE_API Transform {
public:
    Transform();
    Transform(const Transform& other);
    Transform(Transform&& other) noexcept;
    ~Transform() = default;

    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other) noexcept;

    void rotate(const math::Vector3<float>& eulerAngles);
    void scale(const math::Vector3<float>& scale);
    void translate(const math::Vector3<float>& translate);

    math::Matrix4x4<float> getMatrix() const;

private:
    math::Vector3<float> m_scale;
    math::Vector3<float> m_rotate;
    math::Vector3<float> m_translate;
};

}  // namespace engine
