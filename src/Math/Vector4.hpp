#pragma once

#include <mathfu/vector_4.h>

namespace engine {

namespace math {

template <typename T>
class Vector2;
template <typename T>
class Vector3;

template <typename T>
class Vector4 {
public:
    typedef mathfu::Vector<T, 4> data_type;

    inline Vector4() {}

    explicit inline Vector4(const T& s) : data_(s) {}

    inline Vector4(const T& x, const T& y, const T& z, const T& w)
          : data_(x, y, z, w) {}

    inline Vector4(const Vector3<T>& v, const T& w) : data_(v.x, v.y, v.z, w) {}

    inline Vector4(const Vector2<T>& v, const T& z, const T& w)
          : data_(v.x, v.y, z, w) {}

    inline Vector4(const Vector4<T>& v) : data_(v.data_) {}

    template <typename T2>
    explicit inline Vector4(const Vector4<T2>& v) : data_(v.data_) {}

    explicit inline Vector4(const data_type& v) : data_(v) {}

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

    inline T& operator[](const int i) {
        return data_[i];
    }

    inline const T& operator[](const int i) const {
        return data_[i];
    }

    inline Vector4<T> operator-() const {
        return Vector4<T>(-data_);
    }

    inline Vector4<T> operator*(const Vector4<T>& v) const {
        return Vector4<T>(data_ * v.data_);
    }

    inline Vector4<T> operator/(const Vector4<T>& v) const {
        return Vector4<T>(data_ / v.data_);
    }

    inline Vector4<T> operator+(const Vector4<T>& v) const {
        return Vector4<T>(data_ + v.data_);
    }

    inline Vector4<T> operator-(const Vector4<T>& v) const {
        return Vector4<T>(data_ - v.data_);
    }

    inline Vector4<T> operator*(const T& s) const {
        return Vector4<T>(data_ * s);
    }

    inline Vector4<T> operator/(const T& s) const {
        return Vector4<T>(data_ / s);
    }

    inline Vector4<T> operator+(const T& s) const {
        return Vector4<T>(data_ + s);
    }

    inline Vector4<T> operator-(const T& s) const {
        return Vector4<T>(data_ - s);
    }

    inline Vector4<T>& operator*=(const Vector4<T>& v) {
        data_ *= v.data_;
        return *this;
    }

    inline Vector4<T>& operator/=(const Vector4<T>& v) {
        data_ /= v.data_;
        return *this;
    }

    inline Vector4<T>& operator+=(const Vector4<T>& v) {
        data_ += v.data_;
        return *this;
    }

    inline Vector4<T>& operator-=(const Vector4<T>& v) {
        data_ -= v.data_;
        return *this;
    }

    inline Vector4<T>& operator*=(const T& s) {
        data_ *= s;
        return *this;
    }

    inline Vector4<T>& operator/=(const T& s) {
        data_ /= s;
        return *this;
    }

    inline Vector4<T>& operator+=(const T& s) {
        data_ += s;
        return *this;
    }

    inline Vector4<T>& operator-=(const T& s) {
        data_ -= s;
        return *this;
    }

    inline Vector3<T> xyz() const {
        return Vector3<T>(data_.xyz());
    }

    static inline Vector4<T> Lerp(const Vector4<T>& v1, const Vector4<T>& v2,
                                  const T percent) {
        return Vector4<T>(data_type::Lerp(v1.data_, v2.data_, percent));
    }

public:
    union {
        data_type data_;
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
