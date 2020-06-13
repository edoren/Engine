#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class ENGINE_API Time {
public:
    ////////////////////////////////////////////////////
    /// Static member variables
    ////////////////////////////////////////////////////////////
    static const Time sZero;

    ////////////////////////////////////////////////////////////
    /// Constructors
    ////////////////////////////////////////////////////////////
    Time();

    Time(const Time& other);

    ////////////////////////////////////////////////////////////
    /// Static class methods
    ////////////////////////////////////////////////////////////
    static Time FromNanoseconds(int64 nanoseconds);

    static Time FromMicroseconds(int64 microseconds);

    static Time FromMilliseconds(int64 milliseconds);

    static Time FromSeconds(float seconds);

    static Time FromMinutes(float minutes);

    static Time FromHours(float hours);

    ////////////////////////////////////////////////////////////
    /// Class methods
    ////////////////////////////////////////////////////////////
    int64 AsNanoseconds() const;

    int64 AsMicroseconds() const;

    int64 AsMilliseconds() const;

    float AsSeconds() const;

    float AsMinutes() const;

    float AsHours() const;

    ////////////////////////////////////////////////////////////
    /// Operator overload
    ////////////////////////////////////////////////////////////
    friend ENGINE_API bool operator==(const Time& left, const Time& right);

    friend ENGINE_API bool operator!=(const Time& left, const Time& right);

    friend ENGINE_API bool operator<(const Time& left, const Time& right);

    friend ENGINE_API bool operator>(const Time& left, const Time& right);

    friend ENGINE_API bool operator<=(const Time& left, const Time& right);

    friend ENGINE_API bool operator>=(const Time& left, const Time& right);

    friend ENGINE_API Time operator-(const Time& right);

    friend ENGINE_API Time operator+(const Time& left, const Time& right);

    friend ENGINE_API Time& operator+=(Time& left, const Time& right);

    friend ENGINE_API Time operator-(const Time& left, const Time& right);

    friend ENGINE_API Time& operator-=(Time& left, const Time& right);

    friend ENGINE_API Time operator%(const Time& left, const Time& right);

    friend ENGINE_API Time& operator%=(Time& left, const Time& right);

    template <typename T>
    friend Time operator*(Time left, T right) {
        return Time::FromNanoseconds(static_cast<int64>(left.AsNanoseconds() * right));
    }

    template <typename T>
    friend Time operator*(T left, Time right) {
        return Time::FromNanoseconds(static_cast<int64>(left * right.AsNanoseconds()));
    }

    template <typename T>
    friend Time& operator*=(Time& left, T right) {
        left = left * right;
        return left;
    }

    template <typename T>
    friend Time operator/(Time left, T right) {
        return Time::FromNanoseconds(static_cast<int64>(left.AsNanoseconds() / right));
    }

    template <typename T>
    friend Time& operator/=(Time& left, T right) {
        left = left / right;
        return left;
    }

private:
    explicit Time(int64 nanoseconds);

    int64 m_nanoseconds;
};

}  // namespace engine
