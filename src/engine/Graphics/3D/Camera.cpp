#include <Graphics/3D/Camera.hpp>

namespace engine {

const math::vec3 Camera::sWorldUp(0.0F, 1.0F, 0.0F);

Camera::Camera() : position(0.0F, 0.0F, 0.0F), yaw(0.0F), pitch(0.0F), zoom(45.0F) {
    updateCameraVectors();
}

Camera::Camera(const math::vec3& position) : position(position), yaw(0.0F), pitch(0.0F), zoom(45.0F) {
    updateCameraVectors();
}

void Camera::move(const math::vec3& direction) {
    position += direction;
}

void Camera::rotate(float yawDelta, float pitchDelta, bool limitPitch) {
    // Apply euler angles to rotate the pitch
    yaw += yawDelta;
    pitch -= pitchDelta;

    // Limit the pitch movement to avoid strange behaviors
    if (limitPitch) {
        if (pitch > 89.9F) {
            pitch = 89.9F;
        }
        if (pitch < -89.9F) {
            pitch = -89.9F;
        }
    }

    updateCameraVectors();
}

void Camera::rotate(const math::vec2& offset, bool limitPitch) {
    rotate(offset.x, offset.y, limitPitch);
}

void Camera::lookAt(const math::vec3& newPosition) {
    math::vec3 newFront = math::Normalize(newPosition - position);

    pitch = math::Degrees(std::asin(newFront.y));
    yaw = math::Degrees(std::atan2(newFront.z, newFront.x));

    updateCameraVectors();
}

math::mat4 Camera::getViewMatrix() const {
    return math::LookAt(position, position + front, sWorldUp);
}

const math::vec3& Camera::getPosition() const {
    return position;
}

const math::vec3& Camera::getUpVector() const {
    return up;
}

const math::vec3& Camera::getRightVector() const {
    return right;
}

const math::vec3& Camera::getFrontVector() const {
    return front;
}

void Camera::updateCameraVectors() {
    float sinYaw = std::sin(math::Radians(yaw));
    float cosYaw = std::cos(math::Radians(yaw));
    float sinPitch = std::sin(math::Radians(pitch));
    float cosPitch = std::cos(math::Radians(pitch));

    front = math::Normalize(math::vec3(cosPitch * cosYaw, sinPitch, cosPitch * sinYaw));
    right = math::Normalize(math::Cross(front, sWorldUp));
    up = math::Normalize(math::Cross(right, front));
}

}  // namespace engine
