#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Vector4.hpp>

namespace engine {

namespace math {

template <typename T>
class Matrix4x4 {
public:
    typedef mathfu::Matrix<T, 4, 4> data_type;
    typedef Vector4<T> col_type;
    typedef Vector4<T> row_type;
    typedef Matrix4x4<T> type;
    typedef Matrix4x4<T> transpose_type;
    typedef T value_type;

    // Constructors

    inline Matrix4x4() : data_(data_type::Identity()) {}

    explicit inline Matrix4x4(const value_type& s) : data_(s) {}

    inline Matrix4x4(const type& m) : data_(m.data_) {}

    explicit inline Matrix4x4(const data_type& m) : data_(m) {}

    inline Matrix4x4(const value_type& x0, const value_type& x1,
                     const value_type& x2, const value_type& x3,
                     const value_type& y0, const value_type& y1,
                     const value_type& y2, const value_type& y3,
                     const value_type& z0, const value_type& z1,
                     const value_type& z2, const value_type& z3,
                     const value_type& w0, const value_type& w1,
                     const value_type& w2, const value_type& w3)
          : data_(x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3, w0, w1, w2,
                  w3) {}

    inline Matrix4x4(const col_type& column0, const col_type& column1,
                     const col_type& column2, const col_type& column3)
          : Matrix4x4(column0.x, column0.y, column0.z, column0.w, column1.x,
                      column1.y, column1.z, column1.w, column2.x, column2.y,
                      column2.z, column2.w, column3.x, column3.y, column3.z,
                      column3.w) {}

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

    inline type Inverse() const {
        return type(data_.Inverse());
    }

    inline transpose_type Transpose() const {
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
    friend inline typename Matrix4x4<D>::col_type operator*(
        const typename Matrix4x4<D>::row_type& v, const Matrix4x4<D>& m) {
        return typename Matrix4x4<D>::col_type(v.data_ * m.data_);
    }

    template <typename D>
    friend inline typename Matrix4x4<D>::row_type operator*(
        const Matrix4x4<D>& m, const typename Matrix4x4<D>::col_type& v) {
        return typename Matrix4x4<D>::row_type(m.data_ * v.data_);
    }

    // Matrix multiplications

    template <typename D>
    friend inline Matrix4x4<D> operator*(const Matrix4x4<D>& m1,
                                         const Matrix4x4<D>& m2) {
        return Matrix4x4<D>(m1.data_ * m2.data_);
    }

private:
    data_type data_;
};

}  // namespace math

}  // namespace engine
