#pragma once

#include <Util/Prerequisites.hpp>

#include <optional>
#include <vector>

namespace engine {

template <typename T, typename Allocator = std::allocator<T>>
class Vector : public std::vector<T, Allocator> {
public:
    using std::vector<T, Allocator>::vector;
    using std::vector<T, Allocator>::operator=;
    using std::vector<T, Allocator>::operator[];

    Vector(const Vector& other) = default;
    Vector& operator=(const Vector& other) = default;
    Vector(Vector&& other) noexcept = default;
    Vector& operator=(Vector&& other) noexcept = default;

    template <typename Func>
    auto map(Func transform) const -> Vector<std::invoke_result_t<decltype(transform), const T&>>;

    template <typename Func>
    auto mapIndexed(Func transform) const -> Vector<std::invoke_result_t<decltype(transform), size_t, const T&>>;

    template <typename Func>
    auto filter(Func predicate) const -> Vector<T>;

    template <typename Func>
    auto filterIndexed(Func predicate) const -> Vector<T>;

    template <typename Func>
    auto find(Func predicate) -> T*;

    template <typename Func>
    auto find(Func predicate) const -> const T*;

    template <typename Func>
    auto forEach(Func predicate);

    template <typename Func>
    auto forEach(Func predicate) const;

    template <typename Func>
    auto forEachIndexed(Func predicate);

    template <typename Func>
    auto forEachIndexed(Func predicate) const;

    auto first() -> T&;

    auto first() const -> const T&;

    template <typename Func>
    auto first(Func predicate) -> T&;

    template <typename Func>
    auto first(Func predicate) const -> const T&;

    auto firstOrNull() -> T*;

    auto firstOrNull() const -> const T*;

    template <typename Func>
    auto firstOrNull(Func predicate) -> T*;

    template <typename Func>
    auto firstOrNull(Func predicate) const -> const T*;
};

}  // namespace engine

#include <Util/Container/Vector.inl>
