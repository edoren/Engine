#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

namespace math {

template <typename T>
class Vector3;
template <typename T>
class Vector4;

template <typename T>
class Vector2 {
public:
    typedef mathfu::Vector<T, 2> data_type;

    inline Vector2() {}

    explicit inline Vector2(const T& s) : data_(s) {}

    inline Vector2(const T& x, const T& y) : data_(x, y) {}

    inline Vector2(const Vector2<T>& v) : data_(v.data_) {}

    inline Vector2(const Vector3<T>& v) : data_(v.x, v.y) {}

    inline Vector2(const Vector4<T>& v) : data_(v.x, v.y) {}

    template <typename T2>
    explicit inline Vector2(const Vector2<T2>& v) : data_(v.data_) {}

    explicit inline Vector2(const data_type& v) : data_(v) {}

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

    inline T& operator[](const int i) {
        return data_[i];
    }

    inline const T& operator[](const int i) const {
        return data_[i];
    }

    inline Vector2<T> operator-() const {
        return Vector2<T>(-data_);
    }

    inline Vector2<T> operator*(const Vector2<T>& v) const {
        return Vector2<T>(data_ * v.data_);
    }

    inline Vector2<T> operator/(const Vector2<T>& v) const {
        return Vector2<T>(data_ / v.data_);
    }

    inline Vector2<T> operator+(const Vector2<T>& v) const {
        return Vector2<T>(data_ + v.data_);
    }

    inline Vector2<T> operator-(const Vector2<T>& v) const {
        return Vector2<T>(data_ - v.data_);
    }

    inline Vector2<T> operator*(const T& s) const {
        return Vector2<T>(data_ * s);
    }

    inline Vector2<T> operator/(const T& s) const {
        return Vector2<T>(data_ / s);
    }

    inline Vector2<T> operator+(const T& s) const {
        return Vector2<T>(data_ + s);
    }

    inline Vector2<T> operator-(const T& s) const {
        return Vector2<T>(data_ - s);
    }

    inline Vector2<T>& operator*=(const Vector2<T>& v) {
        data_ *= v.data_;
        return *this;
    }

    inline Vector2<T>& operator/=(const Vector2<T>& v) {
        data_ /= v.data_;
        return *this;
    }

    inline Vector2<T>& operator+=(const Vector2<T>& v) {
        data_ += v.data_;
        return *this;
    }

    inline Vector2<T>& operator-=(const Vector2<T>& v) {
        data_ -= v.data_;
        return *this;
    }

    inline Vector2<T>& operator*=(const T& s) {
        data_ *= s;
        return *this;
    }

    inline Vector2<T>& operator/=(const T& s) {
        data_ /= s;
        return *this;
    }

    inline Vector2<T>& operator+=(const T& s) {
        data_ += s;
        return *this;
    }

    inline Vector2<T>& operator-=(const T& s) {
        data_ -= s;
        return *this;
    }

    static inline Vector2<T> Lerp(const Vector2<T>& v1, const Vector2<T>& v2,
                                  const T percent) {
        return Vector2<T>(data_type::Lerp(v1.data_, v2.data_, percent));
    }

public:
    union {
        data_type data_;
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
