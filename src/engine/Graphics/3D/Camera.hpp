#pragma once

#include <Util/Prerequisites.hpp>

#include <Input/InputManager.hpp>

namespace engine {

class ENGINE_API Camera {
public:
    Camera();

    Camera(const math::vec3& position);

    void Move(const math::vec3& direction);

    void Rotate(float yaw, float pitch, bool limit_pitch = true);

    void Rotate(const math::vec2& offset, bool limit_pitch = true);

    void LookAt(const math::vec3& position);

    math::mat4 GetViewMatrix() const;

    const math::vec3& GetPosition() const;

    const math::vec3& GetUpVector() const;
    const math::vec3& GetRightVector() const;
    const math::vec3& GetFrontVector() const;

private:
    void UpdateCameraVectors();

public:
    static math::vec3 WORLD_UP;

    // Camera vectors
    math::vec3 m_position;
    math::vec3 m_up;
    math::vec3 m_right;
    math::vec3 m_front;

    // Euler Angles
    float m_yaw;
    float m_pitch;

    // Camera options
    float m_zoom;
};

}  // namespace engine
