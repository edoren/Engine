#include <Renderer/Transform.hpp>

#include <Math/MatrixTransform.hpp>

namespace engine {

Transform::Transform() : m_matrix() {}

Transform::Transform(const Transform& other) : m_matrix(other.m_matrix) {}

Transform::Transform(Transform&& other) : m_matrix(std::move(other.m_matrix)) {}

Transform::Transform(const math::Matrix4x4<float>& matrix) : m_matrix(matrix) {}

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

const math::Matrix4x4<float>& Transform::GetMatrix() const {
    return m_matrix;
}

void Transform::Rotate(float angle, const math::Vector3<float>& v) {
    m_matrix *= math::Rotate(angle, v);
}

void Transform::Scale(const math::Vector3<float>& v) {
    m_matrix *= math::Scale(v);
}

void Transform::Translate(const math::Vector3<float>& v) {
    m_matrix *= math::Translate(v);
}

}  // namespace engine
