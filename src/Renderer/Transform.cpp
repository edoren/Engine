#include <Renderer/Transform.hpp>

#include <Math/MatrixTransform.hpp>

namespace engine {

Transform::Transform()
      : m_scale(1, 1, 1), m_rotate(0, 0, 0), m_translate(0, 0, 0) {}

Transform::Transform(const Transform& other)
      : m_scale(other.m_scale),
        m_rotate(other.m_rotate),
        m_translate(other.m_translate) {}

Transform::Transform(Transform&& other)
      : m_scale(std::move(other.m_scale)),
        m_rotate(std::move(other.m_rotate)),
        m_translate(std::move(other.m_translate)) {}

Transform::~Transform() {}

Transform& Transform::operator=(const Transform& other) {
    this->~Transform();
    new (this) Transform(other);
    return *this;
}

Transform& Transform::operator=(Transform&& other) {
    new (this) Transform(std::move(other));
    return *this;
}

math::Matrix4x4<float> Transform::GetMatrix() const {
    return math::Translate(m_translate) * math::Scale(m_scale) *
           math::Rotate(m_rotate);
}

void Transform::Rotate(const math::Vector3<float>& euler_angles) {
    m_rotate += euler_angles;
}

void Transform::Scale(const math::Vector3<float>& scale) {
    m_scale *= scale;
}

void Transform::Translate(const math::Vector3<float>& translate) {
    m_translate += translate;
}

}  // namespace engine
