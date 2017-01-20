#pragma once

#include <Util/NonCopyable.hpp>

namespace engine {

template <class T>
class Singleton : NonCopyable {
public:
    Singleton() {
        assert(!instance_);
        instance_ = static_cast<T*>(this);
    }

    ~Singleton() {
        assert(instance_);
        instance_ = nullptr;
    }

    static T& GetInstance() {
        assert(instance_);
        return (*instance_);
    }

    static T* GetInstancePtr() {
        return instance_;
    }

protected:
    static T* instance_;
};

template <class T>
T* Singleton<T>::instance_ = nullptr;

}  // namespace engine
