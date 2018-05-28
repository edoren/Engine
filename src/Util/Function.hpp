#pragma once

#include <cassert>
#include <cstdlib>

#include <type_traits>
#include <utility>

#ifndef LAMBDA_DEFAULT_MAX_SIZE
#define LAMBDA_DEFAULT_MAX_SIZE sizeof(void*) * (6 + 2)
#endif

namespace engine {

template <typename, size_t MaxSize = LAMBDA_DEFAULT_MAX_SIZE>
class Function;

template <typename Ret, typename... Args, size_t MaxSize>
class Function<Ret(Args...), MaxSize> {
public:
    Function() : m_data(), m_invoker(nullptr), m_manager(nullptr) {}

    Function(std::nullptr_t) noexcept
          : m_data(), m_invoker(nullptr), m_manager(nullptr) {}

    Function(const Function& other)
          : m_data(), m_invoker(nullptr), m_manager(nullptr) {
        if (other.m_manager) {
            other.m_manager(&m_data, &other.m_data, Operation::Copy);
            m_invoker = other.m_invoker;
            m_manager = other.m_manager;
        }
    }

    Function(Function&& other)
          : m_data(std::move(other.m_data)),
            m_invoker(std::move(other.m_invoker)),
            m_manager(std::move(other.m_manager)) {
        other.m_invoker = nullptr;
        other.m_manager = nullptr;
    }

    // SFINAE to avoid being called when T is of type Function<Ret(Args...)>
    // This constructor should be called only when T is a lambda or a function
    template <typename T, typename = typename std::enable_if<!std::is_same<
                              typename std::decay<T>::type,
                              Function<Ret(Args...), MaxSize>>::value>::type>
    Function(T&& f) : m_data(), m_invoker(nullptr), m_manager(nullptr) {
        using lambda_type = typename std::decay<T>::type;
        static_assert(alignof(lambda_type) <= alignof(Storage), "");
        static_assert(sizeof(lambda_type) <= sizeof(Storage), "");
        new (&m_data) lambda_type(std::forward<T>(f));
        m_invoker = &CallFunction<lambda_type>;
        m_manager = &ManageFunction<lambda_type>;
    }

    ~Function() {
        if (m_manager != nullptr) {
            m_manager(&m_data, nullptr, Operation::Destroy);
        }
    }

    Function& operator=(const Function& other) {
        Function(other).Swap(*this);
        return *this;
    }

    Function& operator=(Function&& other) {
        Function(std::move(other)).Swap(*this);
        return *this;
    }

    Function& operator=(std::nullptr_t) {
        if (m_manager != nullptr) {
            m_manager(&m_data, nullptr, Operation::Destroy);
            m_manager = nullptr;
            m_invoker = nullptr;
        }
        return *this;
    }

    template <typename T>
    Function& operator=(T&& other) {
        Function(std::forward<T>(other)).Swap(*this);
        return *this;
    }

    template <typename T>
    Function& operator=(std::reference_wrapper<T> other) {
        Function(other).Swap(*this);
        return *this;
    }

    void Swap(Function& other) {
        std::swap(m_data, other.m_data);
        std::swap(m_manager, other.m_manager);
        std::swap(m_invoker, other.m_invoker);
    }

    explicit operator bool() const noexcept {
        return m_manager != nullptr;
    }

    Ret operator()(Args&&... args) {
        if (m_invoker == nullptr) {
            abort();
        }
        return m_invoker(&m_data, std::forward<Args>(args)...);
    }

    Ret operator()(Args&&... args) const {
        if (m_invoker == nullptr) {
            abort();
        }
        return m_invoker(&m_data, std::forward<Args>(args)...);
    }

private:
    enum class Operation { Copy, Destroy };

    template <typename FunctionType>
    static Ret CallFunction(const void* data, Args&&... args) {
        auto callable = static_cast<const FunctionType*>(data);
        return (*callable)(std::forward<Args>(args)...);
    }

    template <typename FunctionType>
    static void ManageFunction(void* dest, const void* src, Operation op) {
        auto dest_cast = static_cast<FunctionType*>(dest);
        switch (op) {
            case Operation::Copy: {
                auto src_cast = static_cast<const FunctionType*>(src);
                new (dest_cast) FunctionType(*src_cast);
                break;
            }
            case Operation::Destroy:
                dest_cast->~FunctionType();
                break;
        }
    }

    using Invoker = Ret (*)(const void*, Args&&...);
    using Manager = void (*)(void*, const void*, Operation);
    using Storage = typename std::aligned_storage<
        MaxSize - sizeof(Invoker) - sizeof(Manager), 8>::type;  // Why 8?

    Storage m_data;     ///< Stores a copy of the Functor
    Invoker m_invoker;  ///< Pointer to the caller function for m_data
    Manager m_manager;  ///< Pointer to the function that manages the m_data
};

}  // namespace engine
