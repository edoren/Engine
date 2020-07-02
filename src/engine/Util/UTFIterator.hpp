#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/UTF.hpp>

#include <iterator>
#include <utility>

namespace engine {

class String;
class StringView;

template <size_t Base, typename T>
class ENGINE_API UTFIterator {
    static_assert((Base == 8) || (Base == 16) || (Base == 32), "Error invalid Base, it should be either 8, 16 or 32");
    static_assert(std::is_pointer<T>::value, "Type should be a pointer");
    static_assert(std::is_integral<std::remove_pointer_t<T>>::value, "Pointed value should be an integer");
    static_assert(sizeof(std::remove_pointer_t<T>) * 8 == Base,
                  "Error invalid value (T), it should has the same value in bits as Base");

    friend class String;
    friend class StringView;

public:
    class ValueRef {
        friend class UTFIterator;

    public:
        constexpr utf::CodeUnit<Base> getUnit() const;

    private:
        constexpr ValueRef(std::pair<T, T>&& value);
        std::pair<T, T> m_value;
    };

    using difference_type = std::ptrdiff_t;
    using value_type = ValueRef;
    using pointer = ValueRef*;
    using reference = ValueRef&;
    using iterator_category = std::forward_iterator_tag;

    constexpr UTFIterator() = default;

    constexpr UTFIterator(const UTFIterator& other) = default;

    ~UTFIterator() = default;

    constexpr UTFIterator& operator=(const UTFIterator& other) = default;

    constexpr reference operator*();

    constexpr pointer operator->();

    constexpr UTFIterator operator+(uint32 num);

    constexpr UTFIterator& operator+=(uint32 num);

    constexpr UTFIterator& operator++();

    constexpr UTFIterator operator++(int);

    constexpr bool operator==(const UTFIterator& other) const;

    constexpr bool operator!=(const UTFIterator& other) const;

    constexpr void swap(UTFIterator& other);

    constexpr T getPtr() const;

private:
    constexpr UTFIterator(std::pair<T, T> maxRange, T begin);

    std::pair<T, T> m_maxRange;
    ValueRef m_ref;
};

}  // namespace engine

#include <Util/UTFIterator.inl>
