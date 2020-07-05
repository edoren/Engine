#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/Function.hpp>

namespace engine {

/**
 * @brief Class used to handle a Signal connection
 */
class ENGINE_API SignalConnection {
public:
    template <typename... Args>
    friend class Signal;

    using IdType = uint64_t;  ///< Type used for the signal connection id

    static const IdType sInvalidId;  ///< ID used when a signal connection is invalid

    /**
     * @brief Default constructor
     */
    SignalConnection();

    /**
     * @brief Copy constructor
     */
    SignalConnection(const SignalConnection& other);

    /**
     * @brief Move constructor
     */
    SignalConnection(SignalConnection&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~SignalConnection();

    /**
     * @brief Copy assignment
     */
    SignalConnection& operator=(const SignalConnection& other);

    /**
     * @brief Move assignment
     */
    SignalConnection& operator=(SignalConnection&& other) noexcept;

    /**
     * @brief Get the Id object
     *
     * @return The ID of the signal connection or [sInvalidId](@ref SignalConnection::sInvalidId) if invalid
     */
    IdType getId() const;

    /**
     * @brief Disconnects this signal
     *
     * After disconnected this signal connection becomes invalid
     */
    void disconnect();

    /**
     * @brief Check if connected to a signal
     *
     * @return `true` if connected to a signal, `false` otherwise
     */
    bool isConnected();

private:
    class Reference;

    using DisconnectCallbackType = Function<void(SignalConnection&), LAMBDA_FUNCTION_SIZE(1)>;

    SignalConnection(uint64_t id, DisconnectCallbackType disconnectCallback);

    void disconnectWithCallback();

    void disconnectWithoutCallback();

    IdType m_id;
    Reference* m_reference;
    DisconnectCallbackType m_disconnectCallback;
};

}  // namespace engine

// #include <System/Signal.inl>
