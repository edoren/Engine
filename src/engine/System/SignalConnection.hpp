#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/Function.hpp>

namespace engine {

class ENGINE_API SignalConnection {
public:
    template <typename... Args>
    friend class Signal;

    using IdType = uint64_t;

private:
    class Reference;

    using DisconnectCallbackType = Function<void(SignalConnection&), LAMBDA_FUNCTION_SIZE(1)>;

    SignalConnection(uint64_t id, DisconnectCallbackType disconnect_callback);

public:
    static const IdType sInvalidId;

    // Default constructor
    SignalConnection();

    // Copy constructor
    SignalConnection(const SignalConnection& other);

    // Move constructor
    SignalConnection(SignalConnection&& other);

    // Destructor
    ~SignalConnection();

    // Copy assignment
    SignalConnection& operator=(const SignalConnection& other);

    // Move assignment
    SignalConnection& operator=(SignalConnection&& other);

    IdType getId() const;

    void disconnect();

    bool isConnected();

private:
    void disconnectWithCallback();

    void disconnectWithoutCallback();

    IdType m_id;
    Reference* m_reference;
    DisconnectCallbackType m_disconnectCallback;
};

}  // namespace engine

// #include <System/Signal.inl>
