#include <System/Time.hpp>

namespace engine {

const Time Time::ZERO = Time(0);

Time::Time() : nanoseconds_(0) {}

Time::Time(const Time& other) : nanoseconds_(other.nanoseconds_) {}

Time::Time(int64 nanoseconds) : nanoseconds_(nanoseconds) {}

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
    return nanoseconds_;
}

int64 Time::AsMicroseconds() const {
    return nanoseconds_ / 1000;
}

int64 Time::AsMilliseconds() const {
    return nanoseconds_ / 1000000;
}

float Time::AsSeconds() const {
    return static_cast<float>(nanoseconds_ / 1000000000.f);
}

float Time::AsMinutes() const {
    return static_cast<float>(nanoseconds_ / 60000000000.f);
}

float Time::AsHours() const {
    return static_cast<float>(nanoseconds_ / 3600000000000.f);
}

bool operator==(const Time& left, const Time& right) {
    return left.nanoseconds_ == right.nanoseconds_;
}

bool operator!=(const Time& left, const Time& right) {
    return left.nanoseconds_ != right.nanoseconds_;
}

bool operator<(const Time& left, const Time& right) {
    return left.nanoseconds_ < right.nanoseconds_;
}

bool operator>(const Time& left, const Time& right) {
    return left.nanoseconds_ > right.nanoseconds_;
}

bool operator<=(const Time& left, const Time& right) {
    return left.nanoseconds_ <= right.nanoseconds_;
}

bool operator>=(const Time& left, const Time& right) {
    return left.nanoseconds_ >= right.nanoseconds_;
}

Time operator-(const Time& right) {
    return Time::FromNanoseconds(-right.nanoseconds_);
}

Time operator+(const Time& left, const Time& right) {
    return Time::FromNanoseconds(left.nanoseconds_ + right.nanoseconds_);
}

Time& operator+=(Time& left, const Time& right) {
    left.nanoseconds_ += right.nanoseconds_;
    return left;
}

Time operator-(const Time& left, const Time& right) {
    return Time::FromNanoseconds(left.nanoseconds_ - right.nanoseconds_);
}

Time& operator-=(Time& left, const Time& right) {
    left.nanoseconds_ -= right.nanoseconds_;
    return left;
}

Time operator%(const Time& left, const Time& right) {
    return Time::FromNanoseconds(left.nanoseconds_ % right.nanoseconds_);
}

Time& operator%=(Time& left, const Time& right) {
    left.nanoseconds_ %= right.nanoseconds_;
    return left;
}

}  // namespace engine
