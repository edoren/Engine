#pragma once

#include <Util/Prerequisites.hpp>

#include <System/SignalConnection.hpp>
#include <Util/Function.hpp>

#include <map>
#include <mutex>
#include <utility>

namespace engine {

/**
 * @brief Class used to manage signal and event connections
 *
 * A signal object may call multiple slots with the
 * same signature. You can connect functions to the signal
 * which will be called when the Emit() method on the
 * signal object is invoked. Any argument passed to Emit()
 * will be passed to the given functions.
 *
 * Example:
 * @code
 * auto fn = [](int a, int b) {
 *     std::cout << a << " + " << b << " = " << a + b << std::endl;
 * };
 * Signal<int, int> sig1;
 * sig1.connect(fn);
 * sig1.emit(2, 3);
 * @endcode
 *
 * @tparam Args The propagated parameters when this signal is emmited
 */
template <typename... Args>
class Signal {
public:
    using SlotType = Function<void(Args...)>;  ///< The stored slot type

    /**
     * @brief Default constructor
     */
    Signal();

    /**
     * @brief Copy constructor
     */
    Signal(const Signal& other);

    /**
     * @brief Move constructor
     */
    Signal(Signal&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~Signal();

    /**
     * @brief Copy assignment
     */
    Signal& operator=(const Signal& other);

    /**
     * @brief Move assignment
     */
    Signal& operator=(Signal&& other) noexcept;

    /**
     * @brief Connects a member function to this Signal
     *
     * Example:
     * @code
     * Signal<> sig;
     * MyClass instance;
     * sig.connect(instance, &MyClass::member);
     * @endcode
     *
     * @param instance Reference to the instance object
     * @param func Member function to connect to
     * @return The signal connection to this slot
     */
    template <typename InstanceType, typename FMInstanceType>
    SignalConnection& connect(InstanceType& instance, void (FMInstanceType::*func)(Args...));

    /**
     * @brief Connects a const member function to this Signal
     *
     * Example:
     * @code
     * Signal<> sig;
     * MyClass instance;
     * sig.connect(instance, &MyClass::constMember);
     * @endcode
     *
     * @param instance Reference to the instance object
     * @param func Const member function to connect to
     * @return The signal connection to this slot
     */
    template <typename InstanceType, typename FMInstanceType>
    SignalConnection& connect(InstanceType& instance, void (FMInstanceType::*func)(Args...) const);

    /**
     * @brief Connects a Slot to this Signal
     *
     * @param slot The function to connect to, it can be a global, static or lambda function
     * @return The signal connection to this slot
     */
    SignalConnection& connect(const SlotType& slot);

    /**
     * @brief Disconnects a previously connected function
     *
     * @param connection The signal connection to disconnect
     */
    void disconnect(SignalConnection& connection);

    /**
     * @brief Disconnects all previously connected functions
     */
    void disconnectAll();

    /**
     * @brief Calls all connected functions
     *
     * @param args The arguments to propagate to the connected functions
     */
    void emit(Args&&... args);

private:
    SignalConnection::IdType m_currentId;
    std::map<SignalConnection::IdType, std::pair<SignalConnection, SlotType>> m_slots;
    mutable std::mutex m_slotsMutex;
};

}  // namespace engine

#include <System/Signal.inl>
