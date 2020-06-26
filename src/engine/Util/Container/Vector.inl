#pragma once

#include <Util/Prerequisites.hpp>

#include <optional>

namespace engine {

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::map(Func transform) const -> Vector<decltype(transform(this->front()))> {
    Vector<decltype(transform(this->front()))> newVec;
    newVec.reserve(this->size());
    for (const auto& element : *this) {
        newVec.push_back(transform(element));
    }
    return newVec;
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::mapIndexed(Func transform) const -> Vector<decltype(transform(this->front()))> {
    Vector<decltype(transform(this->front()))> newVec;
    newVec.reserve(this->size());
    for (decltype(this->size()) i = 0; i < this->size(); i++) {
        newVec.push_back(transform(i, at(i)));
    }
    return newVec;
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::filter(Func predicate) const -> Vector<T> {
    Vector<T> newVec;
    for (const auto& element : *this) {
        if (predicate(element)) {
            newVec.push_back(element);
        }
    }
    return newVec;
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::filterIndexed(Func predicate) const -> Vector<T> {
    Vector<T> newVec;
    for (decltype(this->size()) i = 0; i < this->size(); i++) {
        if (predicate(i, at(i))) {
            newVec.push_back(at(i));
        }
    }
    return newVec;
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::find(Func predicate) const -> std::optional<T> {
    std::optional<T> foundElement;
    auto it = std::find_if(this->cbegin(), this->cend(), predicate);
    if (it != this->cend()) {
        foundElement = *it;
    }
    return foundElement;
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::forEach(Func predicate) {
    for (auto& element : *this) {
        predicate(element);
    }
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::forEach(Func predicate) const {
    for (const auto& element : *this) {
        predicate(element);
    }
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::forEachIndexed(Func predicate) {
    for (decltype(this->size()) i = 0; i < this->size(); i++) {
        predicate(i, this->get(i));
    }
}

template <typename T, typename Allocator>
template <typename Func>
auto Vector<T, Allocator>::forEachIndexed(Func predicate) const {
    for (decltype(this->size()) i = 0; i < this->size(); i++) {
        predicate(i, this->get(i));
    }
}

}  // namespace engine

