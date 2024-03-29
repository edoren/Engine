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
struct Vector2Packed {
    Vector2Packed() : data() {}

    explicit Vector2Packed(const Vector2<T>& vector) {
        vector.pack(this);
    }

    Vector2Packed& operator=(const Vector2<T>& vector) {
        vector.pack(this);
        return *this;
    }

    union {
        T data[2];
        struct {
            T x, y;
        };
    };
};

template <typename T>
class Vector2 {
public:
    using data_type = mathfu::Vector<T, 2>;

    inline Vector2() : data() {}

    explicit inline Vector2(const T& s) : data(s) {}

    inline Vector2(const T& x, const T& y) : data(x, y) {}

    inline Vector2(const Vector2<T>& v) : data(v.data) {}

    inline Vector2(const Vector3<T>& v) : data(v.x, v.y) {}

    inline Vector2(const Vector4<T>& v) : data(v.x, v.y) {}

    template <typename T2>
    explicit inline Vector2(const Vector2<T2>& v) : data(v.data) {}

    explicit inline Vector2(const data_type& v) : data(v) {}

    inline Vector2<T>& operator=(const Vector2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline Vector2<T>& operator=(const Vector3<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline Vector2<T>& operator=(const Vector4<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline T& operator[](const size_t i) {
        return data[static_cast<int>(i)];
    }

    inline const T& operator[](const size_t i) const {
        return data[static_cast<int>(i)];
    }

    inline Vector2<T> operator-() const {
        return Vector2<T>(-data);
    }

    inline Vector2<T> operator*(const Vector2<T>& v) const {
        return Vector2<T>(data * v.data);
    }

    inline Vector2<T> operator/(const Vector2<T>& v) const {
        return Vector2<T>(data / v.data);
    }

    inline Vector2<T> operator+(const Vector2<T>& v) const {
        return Vector2<T>(data + v.data);
    }

    inline Vector2<T> operator-(const Vector2<T>& v) const {
        return Vector2<T>(data - v.data);
    }

    inline Vector2<T> operator*(const T& s) const {
        return Vector2<T>(data * s);
    }

    inline Vector2<T> operator/(const T& s) const {
        return Vector2<T>(data / s);
    }

    inline Vector2<T> operator+(const T& s) const {
        return Vector2<T>(data + s);
    }

    inline Vector2<T> operator-(const T& s) const {
        return Vector2<T>(data - s);
    }

    inline Vector2<T>& operator*=(const Vector2<T>& v) {
        data *= v.data;
        return *this;
    }

    inline Vector2<T>& operator/=(const Vector2<T>& v) {
        data /= v.data;
        return *this;
    }

    inline Vector2<T>& operator+=(const Vector2<T>& v) {
        data += v.data;
        return *this;
    }

    inline Vector2<T>& operator-=(const Vector2<T>& v) {
        data -= v.data;
        return *this;
    }

    inline Vector2<T>& operator*=(const T& s) {
        data *= s;
        return *this;
    }

    inline Vector2<T>& operator/=(const T& s) {
        data /= s;
        return *this;
    }

    inline Vector2<T>& operator+=(const T& s) {
        data += s;
        return *this;
    }

    inline Vector2<T>& operator-=(const T& s) {
        data -= s;
        return *this;
    }

    inline bool operator==(const Vector2<T>& v) {
        return (x == v.x) && (y == v.y);
    }

    inline bool operator!=(const Vector2<T>& v) {
        return !(*this == v);
    }

    inline void pack(Vector2Packed<T>* vector) const {
        vector->x = x;
        vector->y = y;
    }

    static inline Vector2<T> Lerp(const Vector2<T>& v1, const Vector2<T>& v2, const T percent) {
        return Vector2<T>(data_type::Lerp(v1.data, v2.data, percent));
    }

    union {
        data_type data;
        struct {
            T x, y;
        };
    };
};

template <typename T>
inline Vector2<T> operator+(const T& s, const Vector2<T>& v) {
    return v + s;
}

template <typename T>
inline Vector2<T> operator*(const T& s, const Vector2<T>& v) {
    return v * s;
}

template <typename T>
inline Vector2<T> operator/(const Vector2<T>& v, const T& s) {
    return v / s;
}

}  // namespace math

}  // namespace engine
