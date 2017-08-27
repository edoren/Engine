#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Math.hpp>

namespace engine {

class ENGINE_API Vertex {
public:
    Vertex() : m_position(0, 0, 0), m_normal(0, 0, 0), m_tex_coords(0, 0) {}
    Vertex(const math::vec3& position, const math::vec3& normal,
           const math::vec2& texcoords)
          : m_position(position), m_normal(normal), m_tex_coords(texcoords) {}

    void SetPosition(const math::vec3& position) {
        m_position = position;
    }

    const math::vec3& GetPosition() const {
        return m_position;
    }

    void SetNormal(const math::vec3& normal) {
        m_normal = normal;
    }

    const math::vec3& GetNormal() const {
        return m_normal;
    }

    void SetTexCoords(const math::vec2& texcoords) {
        m_tex_coords = texcoords;
    }

    const math::vec2& GetTexCoords() const {
        return m_tex_coords;
    }

    math::vec3 m_position;
    math::vec3 m_normal;
    math::vec2 m_tex_coords;
};

}  // namespace engine
