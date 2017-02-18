#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

namespace engine {

namespace math {

template <typename T>
class Vector2;
template <typename T>
class Vector4;

template <typename T>
class Vector3 {
public:
    typedef mathfu::Vector<T, 3> data_type;

    inline Vector3() {}

    explicit inline Vector3(const T& s) : data_(s) {}

    inline Vector3(const T& x, const T& y, const T& z) : data_(x, y, z) {}

    inline Vector3(const Vector2<T>& v, const T& z) : data_(v.x, v.y, z) {}

    inline Vector3(const Vector3<T>& v) : data_(v.data_) {}

    inline Vector3(const Vector4<T>& v) : data_(v.data_.xyz()) {}

    template <typename T2>
    explicit inline Vector3(const Vector3<T2>& v) : data_(v.data_) {}

    explicit inline Vector3(const data_type& v) : data_(v) {}

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

    inline T& operator[](const int i) {
        return data_[i];
    }

    inline const T& operator[](const int i) const {
        return data_[i];
    }

    inline Vector3<T> operator-() const {
        return Vector3<T>(-data_);
    }

    inline Vector3<T> operator*(const Vector3<T>& v) const {
        return Vector3<T>(data_ * v.data_);
    }

    inline Vector3<T> operator/(const Vector3<T>& v) const {
        return Vector3<T>(data_ / v.data_);
    }

    inline Vector3<T> operator+(const Vector3<T>& v) const {
        return Vector3<T>(data_ + v.data_);
    }

    inline Vector3<T> operator-(const Vector3<T>& v) const {
        return Vector3<T>(data_ - v.data_);
    }

    inline Vector3<T> operator*(const T& s) const {
        return Vector3<T>(data_ * s);
    }

    inline Vector3<T> operator/(const T& s) const {
        return Vector3<T>(data_ / s);
    }

    inline Vector3<T> operator+(const T& s) const {
        return Vector3<T>(data_ + s);
    }

    inline Vector3<T> operator-(const T& s) const {
        return Vector3<T>(data_ - s);
    }

    inline Vector3<T>& operator*=(const Vector3<T>& v) {
        data_ *= v.data_;
        return *this;
    }

    inline Vector3<T>& operator/=(const Vector3<T>& v) {
        data_ /= v.data_;
        return *this;
    }

    inline Vector3<T>& operator+=(const Vector3<T>& v) {
        data_ += v.data_;
        return *this;
    }

    inline Vector3<T>& operator-=(const Vector3<T>& v) {
        data_ -= v.data_;
        return *this;
    }

    inline Vector3<T>& operator*=(const T& s) {
        data_ *= s;
        return *this;
    }

    inline Vector3<T>& operator/=(const T& s) {
        data_ /= s;
        return *this;
    }

    inline Vector3<T>& operator+=(const T& s) {
        data_ += s;
        return *this;
    }

    inline Vector3<T>& operator-=(const T& s) {
        data_ -= s;
        return *this;
    }

    inline Vector2<T> xy() const {
        return Vector2<T>(data_.xy());
    }

    static inline Vector3<T> Lerp(const Vector3<T>& v1, const Vector3<T>& v2,
                                  const T percent) {
        return Vector3<T>(data_type::Lerp(v1.data_, v2.data_, percent));
    }

public:
    union {
        data_type data_;
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
