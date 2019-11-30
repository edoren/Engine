#include <System/Time.hpp>

namespace engine {

const Time Time::ZERO = Time(0);

Time::Time() : m_nanoseconds(0) {}

Time::Time(const Time& other) : m_nanoseconds(other.m_nanoseconds) {}

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
    return Time(static_cast<int64>(seconds * 1000000000.f));
}

Time Time::FromMinutes(float minutes) {
    return Time(static_cast<int64>(minutes * 60000000000.f));
}
Time Time::FromHours(float hours) {
    return Time(static_cast<int64>(hours * 3600000000000.f));
}

int64 Time::AsNanoseconds() const {
    return m_nanoseconds;
}

int64 Time::AsMicroseconds() const {
    return m_nanoseconds / 1000;
}

int64 Time::AsMilliseconds() const {
    return m_nanoseconds / 1000000;
}

float Time::AsSeconds() const {
    return static_cast<float>(m_nanoseconds / 1000000000.f);
}

float Time::AsMinutes() const {
    return static_cast<float>(m_nanoseconds / 60000000000.f);
}

float Time::AsHours() const {
    return static_cast<float>(m_nanoseconds / 3600000000000.f);
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