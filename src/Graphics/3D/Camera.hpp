#pragma once

#include <Input/InputManager.hpp>
#include <Util/Precompiled.hpp>

namespace engine {

class Camera {
public:
    Camera();

    Camera(const math::vec3& position);

    void Move(const math::vec3& direction);

    void Rotate(float yaw, float pitch, bool limit_pitch = true);

    void Rotate(const math::vec2& offset, bool limit_pitch = true);

    void LookAt(const math::vec3& position);

    math::mat4 GetViewMatrix();

    const math::vec3& GetPosition() const;

    const math::vec3& GetUpVector() const;
    const math::vec3& GetRightVector() const;
    const math::vec3& GetFrontVector() const;

private:
    void UpdateCameraVectors();

public:
    static math::vec3 WORLD_UP;

public:
    // Camera vectors
    math::vec3 position_;
    math::vec3 up_;
    math::vec3 right_;
    math::vec3 front_;

    // Euler Angles
    float yaw_;
    float pitch_;

    // Camera options
    float zoom_;
};
}
