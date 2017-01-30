#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Vector3.hpp>

namespace engine {

namespace math {

template <typename T>
class Matrix4x4;

template <typename T>
class Matrix3x3 {
public:
    typedef mathfu::Matrix<T, 3, 3> data_type;
    typedef Vector3<T> col_type;
    typedef Vector3<T> row_type;
    typedef Matrix3x3<T> type;
    typedef Matrix3x3<T> transpose_type;
    typedef T value_type;

    inline Matrix3x3() : data_(data_type::Identity()) {}

    explicit inline Matrix3x3(const value_type& s) : data_(s) {}

    inline Matrix3x3(const Matrix3x3<T>& m) : data_(m.data_) {}

    explicit inline Matrix3x3(const data_type& m) : data_(m) {}

    inline Matrix3x3(const value_type& x0, const value_type& x1,
                     const value_type& x2, const value_type& y0,
                     const value_type& y1, const value_type& y2,
                     const value_type& z0, const value_type& z1,
                     const value_type& z2)
          : data_(x0, x1, x2, y0, y1, y2, z0, z1, z2) {}

    inline Matrix3x3(const col_type& column0, const col_type& column1,
                     const col_type& column2)
          : Matrix3x3(column0.x, column0.y, column0.z, column1.x, column1.y,
                      column1.z, column2.x, column2.y, column2.z) {}

    // Access operators

    inline value_type& operator()(const int row, const int column) {
        return data_(row, column);
    }

    inline const value_type& operator()(const int row, const int column) const {
        return data_(row, column);
    }

    inline value_type& operator()(const int i) {
        return data_[i];
    }

    inline const value_type& operator()(const int i) const {
        return data_[i];
    }

    inline value_type& operator[](const int i) {
        return data_[i];
    }

    inline const value_type& operator[](const int i) const {
        return data_[i];
    }

    // Matrix arithmetic operators

    inline type operator-() const {
        return type(-data_);
    }

    inline type operator+(const type& m) const {
        return type(data_ + m.data_);
    }

    inline type operator-(const type& m) const {
        return type(data_ - m.data_);
    }

    inline type& operator+=(const type& m) {
        data_ += m.data_;
        return *this;
    }

    inline type& operator*=(const type& m) {
        data_ *= m.data_;
        return *this;
    }

    inline type& operator-=(const type& m) {
        data_ -= m.data_;
        return *this;
    }

    inline type& operator*=(const value_type& s) {
        data_ *= s;
        return *this;
    }

    inline type& operator/=(const value_type& s) {
        data_ /= s;
        return *this;
    }

    inline type& operator+=(const value_type& s) {
        data_ += s;
        return *this;
    }

    inline type& operator-=(const value_type& s) {
        data_ -= s;
        return *this;
    }

    // Matrix Functions

    inline type Inverse() {
        return type(data_.Inverse());
    }

    inline transpose_type Transpose() {
        return transpose_type(data_.Transpose());
    }

    // Scalar multiplications

    inline type operator*(const value_type& s) const {
        return type(data_ * s);
    }

    inline type operator/(const value_type& s) const {
        return type(data_ / s);
    }

    inline type operator+(const value_type& s) const {
        return type(data_ + s);
    }

    inline type operator-(const value_type& s) const {
        return type(data_ - s);
    }

    friend inline type operator*(const value_type& s, const type& m) {
        return m * s;
    }

    friend inline type operator/(const value_type& s, const type& m) {
        return m / s;
    }

    // Vector multiplications

    template <typename D>
    friend inline typename Matrix3x3<D>::col_type operator*(
        const typename Matrix3x3<D>::row_type& v, const Matrix3x3<D>& m) {
        return typename Matrix3x3<D>::col_type(v.data_ * m.data_);
    }

    template <typename D>
    friend inline typename Matrix3x3<D>::row_type operator*(
        const Matrix3x3<D>& m, const typename Matrix3x3<D>::col_type& v) {
        return typename Matrix3x3<D>::row_type(m.data_ * v.data_);
    }

    // Matrix multiplications

    template <typename D>
    friend inline Matrix3x3<D> operator*(const Matrix3x3<D>& m1,
                                         const Matrix3x3<D>& m2) {
        return Matrix3x3<D>(m1.data_ * m2.data_);
    }

private:
    data_type data_;
};

}  // namespace math

}  // namespace engine
