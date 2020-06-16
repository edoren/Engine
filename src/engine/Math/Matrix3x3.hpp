#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

#include <Math/Vector3.hpp>

namespace engine {

namespace math {

template <typename T>
class Matrix4x4;

template <typename T>
class Matrix3x3 {
public:
    using data_type = mathfu::Matrix<T, 3, 3>;
    using col_type = Vector3<T>;
    using row_type = Vector3<T>;
    using type = Matrix3x3<T>;
    using transpose_type = Matrix3x3<T>;
    using value_type = T;

    inline Matrix3x3() : m_data(data_type::Identity()) {}

    explicit inline Matrix3x3(const value_type& s) : m_data(s) {}

    inline Matrix3x3(const Matrix3x3<T>& m) : m_data(m.m_data) {}

    explicit inline Matrix3x3(const data_type& m) : m_data(m) {}

    inline Matrix3x3(const value_type& x0,
                     const value_type& x1,
                     const value_type& x2,
                     const value_type& y0,
                     const value_type& y1,
                     const value_type& y2,
                     const value_type& z0,
                     const value_type& z1,
                     const value_type& z2)
          : m_data(x0, x1, x2, y0, y1, y2, z0, z1, z2) {}

    inline Matrix3x3(const col_type& column0, const col_type& column1, const col_type& column2)
          : Matrix3x3(column0.x,
                      column0.y,
                      column0.z,
                      column1.x,
                      column1.y,
                      column1.z,
                      column2.x,
                      column2.y,
                      column2.z) {}

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

    inline type inverse() {
        return type(m_data.Inverse());
    }

    inline transpose_type transpose() {
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
    friend inline typename Matrix3x3<D>::col_type operator*(const typename Matrix3x3<D>::row_type& v,
                                                            const Matrix3x3<D>& m) {
        return typename Matrix3x3<D>::col_type(v.data * m.m_data);
    }

    template <typename D>
    friend inline typename Matrix3x3<D>::row_type operator*(const Matrix3x3<D>& m,
                                                            const typename Matrix3x3<D>::col_type& v) {
        return typename Matrix3x3<D>::row_type(m.m_data * v.data);
    }

    // Matrix multiplications

    template <typename D>
    friend inline Matrix3x3<D> operator*(const Matrix3x3<D>& m1, const Matrix3x3<D>& m2) {
        return Matrix3x3<D>(m1.m_data * m2.m_data);
    }

private:
    data_type m_data;
};

}  // namespace math

}  // namespace engine
