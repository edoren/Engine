#pragma once

#include <mathfu/glsl_mappings.h>  // TMP

#include <Math/Vector2.hpp>

namespace engine {

namespace math {

template <typename T>
class Matrix2x2 {
public:
    typedef mathfu::Matrix<T, 2, 2> data_type;
    typedef Vector2<T> col_type;
    typedef Vector2<T> row_type;
    typedef Matrix2x2<T> type;
    typedef Matrix2x2<T> transpose_type;
    typedef T value_type;

    inline Matrix2x2() : m_data(data_type::Identity()) {}

    explicit inline Matrix2x2(const value_type& s) : m_data(s) {}

    inline Matrix2x2(const Matrix2x2<T>& m) : m_data(m.m_data) {}

    explicit inline Matrix2x2(const data_type& m) : m_data(m) {}

    inline Matrix2x2(const value_type& x0, const value_type& x1, const value_type& y0, const value_type& y1)
          : m_data(x0, x1, y0, y1) {}

    inline Matrix2x2(const col_type& column0, const col_type& column1)
          : Matrix2x2(column0.x, column0.y, column1.x, column1.y) {}

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
    friend inline typename Matrix2x2<D>::col_type operator*(const typename Matrix2x2<D>::row_type& v,
                                                            const Matrix2x2<D>& m) {
        return typename Matrix2x2<D>::col_type(v.m_data * m.m_data);
    }

    template <typename D>
    friend inline typename Matrix2x2<D>::row_type operator*(const Matrix2x2<D>& m,
                                                            const typename Matrix2x2<D>::col_type& v) {
        return typename Matrix2x2<D>::row_type(m.m_data * v.m_data);
    }

    // Matrix multiplications

    template <typename D>
    friend inline Matrix2x2<D> operator*(const Matrix2x2<D>& m1, const Matrix2x2<D>& m2) {
        return Matrix2x2<D>(m1.m_data * m2.m_data);
    }

private:
    data_type m_data;
};

}  // namespace math

}  // namespace engine
