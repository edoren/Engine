#include <Graphics/3D/Camera.hpp>

namespace engine {

math::vec3 Camera::WORLD_UP(0.0f, 1.0f, 0.0f);

Camera::Camera()
      : position_(0.0f, 0.0f, 0.0f), yaw_(0.0f), pitch_(0.0f), zoom_(45.0f) {
    UpdateCameraVectors();
}

Camera::Camera(const math::vec3& position)
      : position_(position), yaw_(0.0f), pitch_(0.0f), zoom_(45.0f) {
    UpdateCameraVectors();
}

math::mat4 Camera::GetViewMatrix() {
    return math::LookAt(position_, position_ + front_, WORLD_UP);
}

void Camera::Move(const math::vec3& direction) {
    position_ += direction;
}

void Camera::Rotate(float yaw, float pitch, bool limit_pitch) {
    // Apply euler angles to rotate the pitch_
    yaw_ += yaw;
    pitch_ -= pitch;

    // Limit the pitch movement to avoid strange behaviors
    if (limit_pitch) {
        if (pitch_ > 89.9f) {
            pitch_ = 89.9f;
        }
        if (pitch_ < -89.9f) {
            pitch_ = -89.9f;
        }
    }

    UpdateCameraVectors();
}

void Camera::Rotate(const math::vec2& offset, bool limit_pitch) {
    Rotate(offset.x, offset.y, limit_pitch);
}

void Camera::LookAt(const math::vec3& position) {
    math::vec3 front = math::Normalize(position - position_);

    pitch_ = math::Degrees(std::asin(front.y));
    yaw_ = math::Degrees(std::atan2(front.z, front.x));

    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() {
    float sin_yaw = std::sin(math::Radians(yaw_));
    float cos_yaw = std::cos(math::Radians(yaw_));
    float sin_pitch = std::sin(math::Radians(pitch_));
    float cos_pitch = std::cos(math::Radians(pitch_));

    front_ = math::Normalize(
        math::vec3(cos_pitch * cos_yaw, sin_pitch, cos_pitch * sin_yaw));
    right_ = math::Normalize(math::Cross(front_, WORLD_UP));
    up_ = math::Normalize(math::Cross(right_, front_));
}

const math::vec3& Camera::GetPosition() const {
    return position_;
}

const math::vec3& Camera::GetUpVector() const {
    return up_;
}

const math::vec3& Camera::GetRightVector() const {
    return right_;
}

const math::vec3& Camera::GetFrontVector() const {
    return front_;
}
}
