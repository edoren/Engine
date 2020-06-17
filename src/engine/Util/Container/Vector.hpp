#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

template <typename T, typename Allocator = std::allocator<T>>
class Vector : public std::vector<T, Allocator> {
public:
    using std::vector<T, Allocator>::vector;
    using std::vector<T, Allocator>::operator=;
    using std::vector<T, Allocator>::operator[];

    template <typename Func>
    auto map(Func transform) const -> Vector<decltype(transform(this->front()))>;

    template <typename Func>
    auto mapIndexed(Func transform) const -> Vector<decltype(transform(this->front()))>;

    template <typename Func>
    auto filter(Func predicate) const -> Vector<T>;

    template <typename Func>
    auto filterIndexed(Func predicate) const -> Vector<T>;

    template <typename Func>
    auto find(Func predicate) const -> std::optional<T>;

    template <typename Func>
    auto forEach(Func predicate);

    template <typename Func>
    auto forEach(Func predicate) const;

    template <typename Func>
    auto forEachIndexed(Func predicate);

    template <typename Func>
    auto forEachIndexed(Func predicate) const;
};

}  // namespace engine

#include <Util/Container/Vector.inl>
