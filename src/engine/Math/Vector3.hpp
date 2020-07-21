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
    Vector3Packed() : data() {}

    explicit Vector3Packed(const Vector3<T>& vector) {
        vector.pack(this);
    }

    Vector3Packed& operator=(const Vector3<T>& vector) {
        vector.pack(this);
        return *this;
    }

    union {
        T data[3];
        struct {
            T x, y, z;
        };
    };
};

template <typename T>
class Vector3 {
public:
    using data_type = mathfu::Vector<T, 3>;

    inline Vector3() : data() {}

    inline Vector3(const T& s) : data(s) {}

    inline Vector3(const T& x, const T& y, const T& z) : data(x, y, z) {}

    inline Vector3(const Vector2<T>& v, const T& z) : data(v.x, v.y, z) {}

    inline Vector3(const Vector3<T>& v) : data(v.data) {}

    inline Vector3(const Vector4<T>& v) : data(v.data.xyz()) {}

    template <typename T2>
    explicit inline Vector3(const Vector3<T2>& v) : data(v.data) {}

    explicit inline Vector3(const data_type& v) : data(v) {}

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
        return data[static_cast<int>(i)];
    }

    inline const T& operator[](const size_t i) const {
        return data[static_cast<int>(i)];
    }

    inline Vector3<T> operator-() const {
        return Vector3<T>(-data);
    }

    inline Vector3<T> operator*(const Vector3<T>& v) const {
        return Vector3<T>(data * v.data);
    }

    inline Vector3<T> operator/(const Vector3<T>& v) const {
        return Vector3<T>(data / v.data);
    }

    inline Vector3<T> operator+(const Vector3<T>& v) const {
        return Vector3<T>(data + v.data);
    }

    inline Vector3<T> operator-(const Vector3<T>& v) const {
        return Vector3<T>(data - v.data);
    }

    inline Vector3<T> operator*(const T& s) const {
        return Vector3<T>(data * s);
    }

    inline Vector3<T> operator/(const T& s) const {
        return Vector3<T>(data / s);
    }

    inline Vector3<T> operator+(const T& s) const {
        return Vector3<T>(data + s);
    }

    inline Vector3<T> operator-(const T& s) const {
        return Vector3<T>(data - s);
    }

    inline Vector3<T>& operator*=(const Vector3<T>& v) {
        data *= v.data;
        return *this;
    }

    inline Vector3<T>& operator/=(const Vector3<T>& v) {
        data /= v.data;
        return *this;
    }

    inline Vector3<T>& operator+=(const Vector3<T>& v) {
        data += v.data;
        return *this;
    }

    inline Vector3<T>& operator-=(const Vector3<T>& v) {
        data -= v.data;
        return *this;
    }

    inline Vector3<T>& operator*=(const T& s) {
        data *= s;
        return *this;
    }

    inline Vector3<T>& operator/=(const T& s) {
        data /= s;
        return *this;
    }

    inline Vector3<T>& operator+=(const T& s) {
        data += s;
        return *this;
    }

    inline Vector3<T>& operator-=(const T& s) {
        data -= s;
        return *this;
    }

    inline bool operator==(const Vector3<T>& v) {
        return (x == v.x) && (y == v.y) && (z == v.z);
    }

    inline bool operator!=(const Vector3<T>& v) {
        return !(*this == v);
    }

    inline Vector2<T> xy() const {
        return Vector2<T>(data.xy());
    }

    inline void pack(Vector3Packed<T>* vector) const {
        vector->x = x;
        vector->y = y;
        vector->z = z;
    }

    static inline Vector3<T> Lerp(const Vector3<T>& v1, const Vector3<T>& v2, const T percent) {
        return Vector3<T>(data_type::Lerp(v1.data, v2.data, percent));
    }

    union {
        data_type data;
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
