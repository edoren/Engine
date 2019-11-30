#pragma once

#include <Util/Prerequisites.hpp>

#include <System/SignalConnection.hpp>
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
    using SlotType = Function<void(Args...)>;

    // Default constructor
    Signal();

    // Copy constructor
    Signal(const Signal& other);

    // Move constructor
    Signal(Signal&& other);

    // Destructor
    ~Signal();

    // Copy assignment
    Signal& operator=(const Signal& other);

    // Move assignment
    Signal& operator=(Signal&& other);

    // Connects a member function to this Signal
    template <typename InstanceType, typename FMInstanceType>
    SignalConnection& Connect(InstanceType& instance, void (FMInstanceType::*func)(Args...));

    // Connects a const member function to this Signal
    template <typename InstanceType, typename FMInstanceType>
    SignalConnection& Connect(InstanceType& instance, void (FMInstanceType::*func)(Args...) const);

    // Connects a Slot to this Signal
    SignalConnection& Connect(const SlotType& slot);

    // Disconnects a previously connected function
    void Disconnect(SignalConnection& connection);

    // Disconnects all previously connected functions
    void DisconnectAll();

    // Calls all connected functions
    void Emit(Args... args);

private:
    SignalConnection::IdType m_current_id;
    std::map<SignalConnection::IdType, std::pair<SignalConnection, SlotType>> m_slots;
    mutable std::mutex m_slots_mutex;
};

}  // namespace engine

#include <System/Signal.inl>
