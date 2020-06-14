#pragma once

#include <Util/Prerequisites.hpp>

#include <Input/InputManager.hpp>

namespace engine {

class ENGINE_API Camera {
public:
    Camera();

    Camera(const math::vec3& position);

    void move(const math::vec3& direction);

    void rotate(float yaw, float pitch, bool limit_pitch = true);

    void rotate(const math::vec2& offset, bool limit_pitch = true);

    void lookAt(const math::vec3& position);

    math::mat4 getViewMatrix() const;

    const math::vec3& getPosition() const;

    const math::vec3& getUpVector() const;
    const math::vec3& getRightVector() const;
    const math::vec3& getFrontVector() const;

private:
    void updateCameraVectors();

public:
    static math::vec3 WORLD_UP;

    // Camera vectors
    math::vec3 position;
    math::vec3 up;
    math::vec3 right;
    math::vec3 front;

    // Euler Angles
    float yaw;
    float pitch;

    // Camera options
    float zoom;
};

}  // namespace engine
