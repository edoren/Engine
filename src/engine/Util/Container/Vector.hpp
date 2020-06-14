#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

template <typename T, typename Allocator = std::allocator<T>>
class Vector : public std::vector<T, Allocator> {
public:
    template <typename Func>
    auto map(Func transform) const -> Vector<decltype(transform(front()))> {
        Vector<decltype(transform(front()))> newVec;
        newVec.reserve(size());
        for (const auto& element : *this) {
            newVec.push_back(transform(element));
        }
        return newVec;
    }

    template <typename Func>
    auto mapIndexed(Func transform) const -> Vector<decltype(transform(front()))> {
        Vector<decltype(transform(front()))> newVec;
        newVec.reserve(size());
        for (decltype(size()) i = 0; i < size(); i++) {
            newVec.push_back(transform(i, at(i)));
        }
        return newVec;
    }

    template <typename Func>
    auto filter(Func predicate) const -> Vector<T> {
        Vector<T> newVec;
        for (const auto& element : *this) {
            if (predicate(element)) {
                newVec.push_back(element);
            }
        }
        return newVec;
    }

    template <typename Func>
    auto filterIndexed(Func predicate) const -> Vector<T> {
        Vector<T> newVec;
        for (decltype(size()) i = 0; i < size(); i++) {
            if (predicate(i, at(i))) {
                newVec.push_back(at(i));
            }
        }
        return newVec;
    }

    template <typename Func>
    auto find(Func predicate) const -> std::optional<T> {
        std::optional<T> foundElement;
        auto it = std::find_if(cbegin(), cend(), predicate);
        if (it != cend()) {
            foundElement = *it;
        }
        return foundElement;
    }
};

}  // namespace engine
