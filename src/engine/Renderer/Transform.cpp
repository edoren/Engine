#include <Renderer/Transform.hpp>

#include <Math/MatrixTransform.hpp>

namespace engine {

Transform::Transform() : m_scale(1, 1, 1), m_rotate(0, 0, 0), m_translate(0, 0, 0) {}

Transform::Transform(const Transform& other)

    = default;

Transform::Transform(Transform&& other) noexcept
      : m_scale(other.m_scale),
        m_rotate(other.m_rotate),
        m_translate(other.m_translate) {}

Transform& Transform::operator=(const Transform& other) {
    new (this) Transform(other);
    return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept {
    new (this) Transform(std::move(other));
    return *this;
}

math::Matrix4x4<float> Transform::getMatrix() const {
    return math::Translate(m_translate) * math::Scale(m_scale) * math::Rotate(m_rotate);
}

void Transform::rotate(const math::Vector3<float>& eulerAngles) {
    m_rotate += eulerAngles;
}

void Transform::scale(const math::Vector3<float>& scale) {
    m_scale *= scale;
}

void Transform::translate(const math::Vector3<float>& translate) {
    m_translate += translate;
}

}  // namespace engine
