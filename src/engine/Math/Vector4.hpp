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
struct Vector4Packed {
    typedef mathfu::VectorPacked<T, 4> data_type;

    Vector4Packed() {}

    explicit Vector4Packed(const Vector4<T>& vector) {
        vector.Pack(this);
    }

    Vector4Packed& operator=(const Vector4<T>& vector) {
        vector.Pack(this);
        return *this;
    }

    union {
        data_type m_data;
        struct {
            T x, y, z, w;
        };
    };
};

template <typename T>
class Vector4 {
public:
    typedef mathfu::Vector<T, 4> data_type;

    inline Vector4() {}

    explicit inline Vector4(const T& s) : m_data(s) {}

    inline Vector4(const T& x, const T& y, const T& z, const T& w) : m_data(x, y, z, w) {}

    inline Vector4(const Vector3<T>& v, const T& w) : m_data(v.x, v.y, v.z, w) {}

    inline Vector4(const Vector2<T>& v, const T& z, const T& w) : m_data(v.x, v.y, z, w) {}

    inline Vector4(const Vector4<T>& v) : m_data(v.m_data) {}

    template <typename T2>
    explicit inline Vector4(const Vector4<T2>& v) : m_data(v.m_data) {}

    explicit inline Vector4(const data_type& v) : m_data(v) {}

    inline Vector4<T>& operator=(const Vector2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline Vector4<T>& operator=(const Vector3<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    inline Vector4<T>& operator=(const Vector4<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
        return *this;
    }

    inline T& operator[](const size_t i) {
        return m_data[static_cast<int>(i)];
    }

    inline const T& operator[](const size_t i) const {
        return m_data[static_cast<int>(i)];
    }

    inline Vector4<T> operator-() const {
        return Vector4<T>(-m_data);
    }

    inline Vector4<T> operator*(const Vector4<T>& v) const {
        return Vector4<T>(m_data * v.m_data);
    }

    inline Vector4<T> operator/(const Vector4<T>& v) const {
        return Vector4<T>(m_data / v.m_data);
    }

    inline Vector4<T> operator+(const Vector4<T>& v) const {
        return Vector4<T>(m_data + v.m_data);
    }

    inline Vector4<T> operator-(const Vector4<T>& v) const {
        return Vector4<T>(m_data - v.m_data);
    }

    inline Vector4<T> operator*(const T& s) const {
        return Vector4<T>(m_data * s);
    }

    inline Vector4<T> operator/(const T& s) const {
        return Vector4<T>(m_data / s);
    }

    inline Vector4<T> operator+(const T& s) const {
        return Vector4<T>(m_data + s);
    }

    inline Vector4<T> operator-(const T& s) const {
        return Vector4<T>(m_data - s);
    }

    inline Vector4<T>& operator*=(const Vector4<T>& v) {
        m_data *= v.m_data;
        return *this;
    }

    inline Vector4<T>& operator/=(const Vector4<T>& v) {
        m_data /= v.m_data;
        return *this;
    }

    inline Vector4<T>& operator+=(const Vector4<T>& v) {
        m_data += v.m_data;
        return *this;
    }

    inline Vector4<T>& operator-=(const Vector4<T>& v) {
        m_data -= v.m_data;
        return *this;
    }

    inline Vector4<T>& operator*=(const T& s) {
        m_data *= s;
        return *this;
    }

    inline Vector4<T>& operator/=(const T& s) {
        m_data /= s;
        return *this;
    }

    inline Vector4<T>& operator+=(const T& s) {
        m_data += s;
        return *this;
    }

    inline Vector4<T>& operator-=(const T& s) {
        m_data -= s;
        return *this;
    }

    inline bool operator==(const Vector3<T>& v) {
        return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
    }

    inline bool operator!=(const Vector3<T>& v) {
        return !(*this == v);
    }

    inline Vector3<T> xyz() const {
        return Vector3<T>(m_data.xyz());
    }

    inline void Pack(Vector4Packed<T>* vector) const {
        vector->x = x;
        vector->y = y;
        vector->z = z;
        vector->w = w;
    }

    static inline Vector4<T> Lerp(const Vector4<T>& v1, const Vector4<T>& v2, const T percent) {
        return Vector4<T>(data_type::Lerp(v1.m_data, v2.m_data, percent));
    }

    union {
        data_type m_data;
        struct {
            T x, y, z, w;
        };
    };
};

template <typename T>
inline Vector4<T> operator+(const T& s, const Vector4<T>& v) {
    return v + s;
}

template <typename T>
inline Vector4<T> operator*(const T& s, const Vector4<T>& v) {
    return v * s;
}

template <typename T>
inline Vector4<T> operator/(const Vector4<T>& v, const T& s) {
    return v / s;
}

}  // namespace math

}  // namespace engine
