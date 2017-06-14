#pragma once

namespace engine {

template <class T>
class Singleton {
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

    Singleton(const Singleton<T>&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;
};

}  // namespace engine
