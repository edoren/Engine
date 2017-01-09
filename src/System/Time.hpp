#pragma once

#include <chrono>

#include <Util/Precompiled.hpp>

namespace engine {

class Time {
public:
    Time() : nanoseconds_(0) {}

    Time(const Time& other) : nanoseconds_(other.nanoseconds_) {}

    int64 AsNanoseconds() {
        return nanoseconds_;
    }

    int64 AsMicroseconds() {
        return nanoseconds_ / 1000;
    }

    int64 AsMilliseconds() {
        return nanoseconds_ / 1000000;
    }

    float AsSeconds() {
        return static_cast<float>(nanoseconds_ / 1000000000.f);
    }

    float AsMinutes() {
        return static_cast<float>(nanoseconds_ / 60000000000.f);
    }

    float AsHours() {
        return static_cast<float>(nanoseconds_ / 3600000000000.f);
    }

    static Time FromNanoseconds(int64 nanoseconds) {
        return Time(nanoseconds);
    }

    static Time FromMicroseconds(int64 microseconds) {
        return Time(microseconds * 1000);
    }

    static Time FromMilliseconds(int64 milliseconds) {
        return Time(milliseconds * 1000000);
    }

    static Time FromSeconds(float seconds) {
        return Time(static_cast<int64>(seconds * 1000000000.f));
    }

    static Time FromMinutes(float minutes) {
        return Time(static_cast<int64>(minutes * 60000000000.f));
    }
    static Time FromHours(float hours) {
        return Time(static_cast<int64>(hours * 3600000000000.f));
    }

private:
    explicit Time(int64 nanoseconds) : nanoseconds_(nanoseconds) {}

public:
    static const Time ZERO;

private:
    int64 nanoseconds_;
};

const Time Time::ZERO = Time(0);

bool operator==(Time left, Time right) {
    return left.AsNanoseconds() == right.AsNanoseconds();
}

bool operator!=(Time left, Time right) {
    return left.AsNanoseconds() != right.AsNanoseconds();
}

bool operator<(Time left, Time right) {
    return left.AsNanoseconds() < right.AsNanoseconds();
}

bool operator>(Time left, Time right) {
    return left.AsNanoseconds() > right.AsNanoseconds();
}

bool operator<=(Time left, Time right) {
    return left.AsNanoseconds() <= right.AsNanoseconds();
}

bool operator>=(Time left, Time right) {
    return left.AsNanoseconds() >= right.AsNanoseconds();
}

Time operator-(Time right) {
    return Time::FromNanoseconds(-right.AsNanoseconds());
}

Time operator+(Time left, Time right) {
    return Time::FromNanoseconds(left.AsNanoseconds() + right.AsNanoseconds());
}

Time& operator+=(Time& left, Time right) {
    left = left + right;
    return left;
}

Time operator-(Time left, Time right) {
    return Time::FromNanoseconds(left.AsNanoseconds() - right.AsNanoseconds());
}

Time& operator-=(Time& left, Time right) {
    left = left - right;
    return left;
}

template <typename T>
Time operator*(Time left, T right) {
    return Time::FromNanoseconds(
        static_cast<int64>(left.AsNanoseconds() * right));
}

template <typename T>
Time operator*(T left, Time right) {
    return Time::FromNanoseconds(
        static_cast<int64>(left * right.AsNanoseconds()));
}

template <typename T>
Time& operator*=(Time& left, T right) {
    left = left * right;
    return left;
}

template <typename T>
Time operator/(Time left, T right) {
    return Time::FromNanoseconds(
        static_cast<int64>(left.AsNanoseconds() / right));
}

template <typename T>
Time& operator/=(Time& left, T right) {
    left = left / right;
    return left;
}

Time operator%(Time left, Time right) {
    return Time::FromNanoseconds(left.AsNanoseconds() % right.AsNanoseconds());
}

Time& operator%=(Time& left, Time right) {
    left = left % right;
    return left;
}

}  // namespace engine
