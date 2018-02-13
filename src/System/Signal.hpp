#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/Function.hpp>

// A signal object may call multiple slots with the
// same signature. You can connect functions to the signal
// which will be called when the Emit() method on the
// signal object is invoked. Any argument passed to Emit()
// will be passed to the given functions.

namespace engine {

template <typename... Args>
class Signal {
public:
    template <typename... Args2>
    using Slot = Function<void(Args2...)>;

    Signal();

    ~Signal();

    // Connects a member function to this Signal
    template <typename T>
    uint32 Connect(T* inst, void (T::*func)(Args...));

    // Connects a const member function to this Signal
    template <typename T>
    uint32 Connect(T* inst, void (T::*func)(Args...) const);

    // Connects a Function to the signal. The returned
    // value can be used to disconnect the function again
    uint32 Connect(const Slot<Args...>& slot);

    // Disconnects a previously connected function
    void Disconnect(uint32 id);

    // Disconnects all previously connected functions
    void DisconnectAll();

    // Calls all connected functions
    void Emit(Args... args);

private:
    std::atomic<uint32> m_current_id;
    std::map<uint32, Slot<Args...>> m_slots;
};

}  // namespace engine

#include <System/Signal.inl>
