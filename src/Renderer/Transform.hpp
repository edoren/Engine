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
    Transform(const math::Matrix4x4<float>& matrix);
    ~Transform();

    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other);

    void Rotate(float angle, const math::Vector3<float>& v);
    void Scale(const math::Vector3<float>& v);
    void Translate(const math::Vector3<float>& v);

    const math::Matrix4x4<float>& GetMatrix() const;

private:
    math::Matrix4x4<float> m_matrix;
};

}  // namespace engine
