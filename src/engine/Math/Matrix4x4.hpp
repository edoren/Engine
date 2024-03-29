#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

#include <Math/Vector4.hpp>

namespace engine {

namespace math {

template <typename T>
class Matrix4x4 {
public:
    using data_type = mathfu::Matrix<T, 4, 4>;
    using col_type = Vector4<T>;
    using row_type = Vector4<T>;
    using type = Matrix4x4<T>;
    using transpose_type = Matrix4x4<T>;
    using value_type = T;

    // Constructors

    inline Matrix4x4() : m_data(data_type::Identity()) {}

    explicit inline Matrix4x4(const value_type& s) : m_data(s) {}

    inline Matrix4x4(const type& m) : m_data(m.m_data) {}

    explicit inline Matrix4x4(const data_type& m) : m_data(m) {}

    inline Matrix4x4(const value_type& x0,
                     const value_type& x1,
                     const value_type& x2,
                     const value_type& x3,
                     const value_type& y0,
                     const value_type& y1,
                     const value_type& y2,
                     const value_type& y3,
                     const value_type& z0,
                     const value_type& z1,
                     const value_type& z2,
                     const value_type& z3,
                     const value_type& w0,
                     const value_type& w1,
                     const value_type& w2,
                     const value_type& w3)
          : m_data(x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3, w0, w1, w2, w3) {}

    inline Matrix4x4(const col_type& column0, const col_type& column1, const col_type& column2, const col_type& column3)
          : Matrix4x4(column0.x,
                      column0.y,
                      column0.z,
                      column0.w,
                      column1.x,
                      column1.y,
                      column1.z,
                      column1.w,
                      column2.x,
                      column2.y,
                      column2.z,
                      column2.w,
                      column3.x,
                      column3.y,
                      column3.z,
                      column3.w) {}

    type& operator=(const type& m) = default;

    // Access operators

    inline value_type& operator()(const size_t row, const size_t column) {
        return m_data(static_cast<int>(row), static_cast<int>(column));
    }

    inline const value_type& operator()(const size_t row, const size_t column) const {
        return m_data(static_cast<int>(row), static_cast<int>(column));
    }

    inline value_type& operator()(const size_t i) {
        return m_data[static_cast<int>(i)];
    }

    inline const value_type& operator()(const size_t i) const {
        return m_data[static_cast<int>(i)];
    }

    inline value_type& operator[](const size_t i) {
        return m_data[static_cast<int>(i)];
    }

    inline const value_type& operator[](const size_t i) const {
        return m_data[static_cast<int>(i)];
    }

    // Matrix arithmetic operators

    inline type operator-() const {
        return type(-m_data);
    }

    inline type operator+(const type& m) const {
        return type(m_data + m.m_data);
    }

    inline type operator-(const type& m) const {
        return type(m_data - m.m_data);
    }

    inline type& operator+=(const type& m) {
        m_data += m.m_data;
        return *this;
    }

    inline type& operator*=(const type& m) {
        m_data *= m.m_data;
        return *this;
    }

    inline type& operator-=(const type& m) {
        m_data -= m.m_data;
        return *this;
    }

    inline type& operator*=(const value_type& s) {
        m_data *= s;
        return *this;
    }

    inline type& operator/=(const value_type& s) {
        m_data /= s;
        return *this;
    }

    inline type& operator+=(const value_type& s) {
        m_data += s;
        return *this;
    }

    inline type& operator-=(const value_type& s) {
        m_data -= s;
        return *this;
    }

    // Matrix Functions

    inline type inverse() const {
        return type(m_data.Inverse());
    }

    inline transpose_type transpose() const {
        return transpose_type(m_data.Transpose());
    }

    // Scalar multiplications

    inline type operator*(const value_type& s) const {
        return type(m_data * s);
    }

    inline type operator/(const value_type& s) const {
        return type(m_data / s);
    }

    inline type operator+(const value_type& s) const {
        return type(m_data + s);
    }

    inline type operator-(const value_type& s) const {
        return type(m_data - s);
    }

    friend inline type operator*(const value_type& s, const type& m) {
        return m * s;
    }

    friend inline type operator/(const value_type& s, const type& m) {
        return m / s;
    }

    // Vector multiplications

    template <typename D>
    friend inline typename Matrix4x4<D>::col_type operator*(const typename Matrix4x4<D>::row_type& v,
                                                            const Matrix4x4<D>& m) {
        return typename Matrix4x4<D>::col_type(v.data * m.m_data);
    }

    template <typename D>
    friend inline typename Matrix4x4<D>::row_type operator*(const Matrix4x4<D>& m,
                                                            const typename Matrix4x4<D>::col_type& v) {
        return typename Matrix4x4<D>::row_type(m.m_data * v.data);
    }

    // Matrix multiplications

    template <typename D>
    friend inline Matrix4x4<D> operator*(const Matrix4x4<D>& m1, const Matrix4x4<D>& m2) {
        return Matrix4x4<D>(m1.m_data * m2.m_data);
    }

private:
    data_type m_data;
};

}  // namespace math

}  // namespace engine
