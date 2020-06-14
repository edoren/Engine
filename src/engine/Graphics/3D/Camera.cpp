#include <Graphics/3D/Camera.hpp>

namespace engine {

math::vec3 Camera::WORLD_UP(0.0F, 1.0F, 0.0F);

Camera::Camera() : m_position(0.0F, 0.0F, 0.0F), m_yaw(0.0F), m_pitch(0.0F), m_zoom(45.0F) {
    updateCameraVectors();
}

Camera::Camera(const math::vec3& position) : m_position(position), m_yaw(0.0F), m_pitch(0.0F), m_zoom(45.0F) {
    updateCameraVectors();
}

void Camera::move(const math::vec3& direction) {
    m_position += direction;
}

void Camera::rotate(float yaw, float pitch, bool limit_pitch) {
    // Apply euler angles to rotate the pitch
    m_yaw += yaw;
    m_pitch -= pitch;

    // Limit the pitch movement to avoid strange behaviors
    if (limit_pitch) {
        if (m_pitch > 89.9F) {
            m_pitch = 89.9F;
        }
        if (m_pitch < -89.9F) {
            m_pitch = -89.9F;
        }
    }

    updateCameraVectors();
}

void Camera::rotate(const math::vec2& offset, bool limit_pitch) {
    rotate(offset.x, offset.y, limit_pitch);
}

void Camera::lookAt(const math::vec3& position) {
    math::vec3 front = math::Normalize(position - m_position);

    m_pitch = math::Degrees(std::asin(front.y));
    m_yaw = math::Degrees(std::atan2(front.z, front.x));

    updateCameraVectors();
}

math::mat4 Camera::getViewMatrix() const {
    return math::LookAt(m_position, m_position + m_front, WORLD_UP);
}

const math::vec3& Camera::getPosition() const {
    return m_position;
}

const math::vec3& Camera::getUpVector() const {
    return m_up;
}

const math::vec3& Camera::getRightVector() const {
    return m_right;
}

const math::vec3& Camera::getFrontVector() const {
    return m_front;
}

void Camera::updateCameraVectors() {
    float sin_yaw = std::sin(math::Radians(m_yaw));
    float cos_yaw = std::cos(math::Radians(m_yaw));
    float sin_pitch = std::sin(math::Radians(m_pitch));
    float cos_pitch = std::cos(math::Radians(m_pitch));

    m_front = math::Normalize(math::vec3(cos_pitch * cos_yaw, sin_pitch, cos_pitch * sin_yaw));
    m_right = math::Normalize(math::Cross(m_front, WORLD_UP));
    m_up = math::Normalize(math::Cross(m_right, m_front));
}

}  // namespace engine
