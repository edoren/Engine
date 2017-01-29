#pragma once

#include <Util/NonCopyable.hpp>

namespace engine {

template <class T>
class ENGINE_API Singleton : NonCopyable {
public:
    Singleton() {
        assert(!s_instance);
        s_instance = static_cast<T*>(this);
    }

    ~Singleton() {
        assert(s_instance);
        s_instance = nullptr;
    }

    static T& GetInstance() {
        assert(s_instance);
        return (*s_instance);
    }

    static T* GetInstancePtr() {
        return s_instance;
    }

protected:
    static T* s_instance;
};

}  // namespace engine
