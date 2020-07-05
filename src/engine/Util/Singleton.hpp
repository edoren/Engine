#pragma once

#include <cassert>

namespace engine {

template <class T>
class Singleton {
public:
    Singleton() {
        ENGINE_ASSERT(!sInstance, "Instance not found");
        sInstance = static_cast<T*>(this);
    }

    ~Singleton() {
        ENGINE_ASSERT(sInstance, "Instance not found");
        sInstance = nullptr;
    }

    static T& GetInstance() {
        ENGINE_ASSERT(sInstance, "Instance not found");
        return (*sInstance);
    }

    static T* GetInstancePtr() {
        return sInstance;
    }

    Singleton(const Singleton<T>&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;

protected:
    static inline T* sInstance = nullptr;
};

}  // namespace engine
