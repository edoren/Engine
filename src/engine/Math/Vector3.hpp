#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

namespace engine {

namespace math {

template <typename T>
class Vector2;
template <typename T>
class Vector3;
template <typename T>
class Vector4;

template <typename T>
struct Vector3Packed {
    typedef mathfu::VectorPacked<T, 3> data_type;

    Vector3Packed() {}

    explicit Vector3Packed(const Vector3<T>& vector) {
        vector.pack(this);
    }

    Vector3Packed& operator=(const Vector3<T>& vector) {
        vector.pack(this);
        return *this;
    }

    union {
        data_type m_data;
        struct {
            T x, y, z;
        };
    };
};

template <typename T>
class Vector3 {
public:
    typedef mathfu::Vector<T, 3> data_type;

    inline Vector3() {}

    explicit inline Vector3(const T& s) : m_data(s) {}

    inline Vector3(const T& x, const T& y, const T& z) : m_data(x, y, z) {}

    inline Vector3(const Vector2<T>& v, const T& z) : m_data(v.x, v.y, z) {}

    inline Vector3(const Vector3<T>& v) : m_data(v.m_data) {}

    inline Vector3(const Vector4<T>& v) : m_data(v.m_data.xyz()) {}

    template <typename T2>
    explicit inline Vector3(const Vector3<T2>& v) : m_data(v.m_data) {}

    explicit inline Vector3(const data_type& v) : m_data(v) {}

    inline Vector3<T>& operator=(const Vector2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline Vector3<T>& operator=(const Vector3<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    inline Vector3<T>& operator=(const Vector4<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    inline T& operator[](const size_t i) {
        return m_data[static_cast<int>(i)];
    }

    inline const T& operator[](const size_t i) const {
        return m_data[static_cast<int>(i)];
    }

    inline Vector3<T> operator-() const {
        return Vector3<T>(-m_data);
    }

    inline Vector3<T> operator*(const Vector3<T>& v) const {
        return Vector3<T>(m_data * v.m_data);
    }

    inline Vector3<T> operator/(const Vector3<T>& v) const {
        return Vector3<T>(m_data / v.m_data);
    }

    inline Vector3<T> operator+(const Vector3<T>& v) const {
        return Vector3<T>(m_data + v.m_data);
    }

    inline Vector3<T> operator-(const Vector3<T>& v) const {
        return Vector3<T>(m_data - v.m_data);
    }

    inline Vector3<T> operator*(const T& s) const {
        return Vector3<T>(m_data * s);
    }

    inline Vector3<T> operator/(const T& s) const {
        return Vector3<T>(m_data / s);
    }

    inline Vector3<T> operator+(const T& s) const {
        return Vector3<T>(m_data + s);
    }

    inline Vector3<T> operator-(const T& s) const {
        return Vector3<T>(m_data - s);
    }

    inline Vector3<T>& operator*=(const Vector3<T>& v) {
        m_data *= v.m_data;
        return *this;
    }

    inline Vector3<T>& operator/=(const Vector3<T>& v) {
        m_data /= v.m_data;
        return *this;
    }

    inline Vector3<T>& operator+=(const Vector3<T>& v) {
        m_data += v.m_data;
        return *this;
    }

    inline Vector3<T>& operator-=(const Vector3<T>& v) {
        m_data -= v.m_data;
        return *this;
    }

    inline Vector3<T>& operator*=(const T& s) {
        m_data *= s;
        return *this;
    }

    inline Vector3<T>& operator/=(const T& s) {
        m_data /= s;
        return *this;
    }

    inline Vector3<T>& operator+=(const T& s) {
        m_data += s;
        return *this;
    }

    inline Vector3<T>& operator-=(const T& s) {
        m_data -= s;
        return *this;
    }

    inline bool operator==(const Vector3<T>& v) {
        return (x == v.x) && (y == v.y) && (z == v.z);
    }

    inline bool operator!=(const Vector3<T>& v) {
        return !(*this == v);
    }

    inline Vector2<T> xy() const {
        return Vector2<T>(m_data.xy());
    }

    inline void pack(Vector3Packed<T>* vector) const {
        vector->x = x;
        vector->y = y;
        vector->z = z;
    }

    static inline Vector3<T> Lerp(const Vector3<T>& v1, const Vector3<T>& v2, const T percent) {
        return Vector3<T>(data_type::Lerp(v1.m_data, v2.m_data, percent));
    }

    union {
        data_type m_data;
        struct {
            T x, y, z;
        };
    };
};

template <typename T>
inline Vector3<T> operator+(const T& s, const Vector3<T>& v) {
    return v + s;
}

template <typename T>
inline Vector3<T> operator*(const T& s, const Vector3<T>& v) {
    return v * s;
}

template <typename T>
inline Vector3<T> operator/(const Vector3<T>& v, const T& s) {
    return v / s;
}

}  // namespace math

}  // namespace engine
