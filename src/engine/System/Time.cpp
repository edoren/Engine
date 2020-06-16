#include <System/Time.hpp>

namespace engine {

const Time Time::sZero = Time(0);

Time::Time() : m_nanoseconds(0) {}

Time::Time(const Time& other) = default;

Time::Time(int64 nanoseconds) : m_nanoseconds(nanoseconds) {}

Time Time::FromNanoseconds(int64 nanoseconds) {
    return Time(nanoseconds);
}

Time Time::FromMicroseconds(int64 microseconds) {
    return Time(microseconds * 1000);
}

Time Time::FromMilliseconds(int64 milliseconds) {
    return Time(milliseconds * 1000000);
}

Time Time::FromSeconds(float seconds) {
    return Time(static_cast<int64>(seconds * 1000000000.F));
}

Time Time::FromMinutes(float minutes) {
    return Time(static_cast<int64>(minutes * 60000000000.F));
}
Time Time::FromHours(float hours) {
    return Time(static_cast<int64>(hours * 3600000000000.F));
}

int64 Time::asNanoseconds() const {
    return m_nanoseconds;
}

int64 Time::asMicroseconds() const {
    return m_nanoseconds / 1000;
}

int64 Time::asMilliseconds() const {
    return m_nanoseconds / 1000000;
}

float Time::asSeconds() const {
    return static_cast<float>(m_nanoseconds / 1000000000.F);
}

float Time::asMinutes() const {
    return static_cast<float>(m_nanoseconds / 60000000000.F);
}

float Time::asHours() const {
    return static_cast<float>(m_nanoseconds / 3600000000000.F);
}

bool operator==(const Time& left, const Time& right) {
    return left.m_nanoseconds == right.m_nanoseconds;
}

bool operator!=(const Time& left, const Time& right) {
    return left.m_nanoseconds != right.m_nanoseconds;
}

bool operator<(const Time& left, const Time& right) {
    return left.m_nanoseconds < right.m_nanoseconds;
}

bool operator>(const Time& left, const Time& right) {
    return left.m_nanoseconds > right.m_nanoseconds;
}

bool operator<=(const Time& left, const Time& right) {
    return left.m_nanoseconds <= right.m_nanoseconds;
}

bool operator>=(const Time& left, const Time& right) {
    return left.m_nanoseconds >= right.m_nanoseconds;
}

Time operator-(const Time& right) {
    return Time::FromNanoseconds(-right.m_nanoseconds);
}

Time operator+(const Time& left, const Time& right) {
    return Time::FromNanoseconds(left.m_nanoseconds + right.m_nanoseconds);
}

Time& operator+=(Time& left, const Time& right) {
    left.m_nanoseconds += right.m_nanoseconds;
    return left;
}

Time operator-(const Time& left, const Time& right) {
    return Time::FromNanoseconds(left.m_nanoseconds - right.m_nanoseconds);
}

Time& operator-=(Time& left, const Time& right) {
    left.m_nanoseconds -= right.m_nanoseconds;
    return left;
}

Time operator%(const Time& left, const Time& right) {
    return Time::FromNanoseconds(left.m_nanoseconds % right.m_nanoseconds);
}

Time& operator%=(Time& left, const Time& right) {
    left.m_nanoseconds %= right.m_nanoseconds;
    return left;
}

}  // namespace engine
