#pragma once

#include <cassert>

namespace engine {

template <class T>
class Singleton {
public:
    Singleton() {
        assert(!sInstance);
        sInstance = static_cast<T*>(this);
    }

    ~Singleton() {
        assert(sInstance);
        sInstance = nullptr;
    }

    static T& GetInstance() {
        assert(sInstance);
        return (*sInstance);
    }

    static T* GetInstancePtr() {
        return sInstance;
    }

protected:
    static T* sInstance;

    Singleton(const Singleton<T>&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;
};

}  // namespace engine
