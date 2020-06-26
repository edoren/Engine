#include <System/SignalConnection.hpp>

#include <mutex>
#include <thread>

namespace engine {

class SignalConnection::Reference {
public:
    Reference(uint64_t count) : m_count(count), m_isConnected(true) {}

    uint64_t increase() {
        std::lock_guard<std::mutex> lock(m_refMutex);
        return ++m_count;
    }

    uint64_t decrease() {
        std::lock_guard<std::mutex> lock(m_refMutex);
        return --m_count;
    }

    uint64_t getCount() const {
        return m_count;
    }

    void disconnect() {
        if (isConnected()) {
            std::lock_guard<std::mutex> lock(m_refMutex);
            m_isConnected = false;
        }
    }

    bool isConnected() const {
        return m_isConnected;
    }

private:
    uint64_t m_count;
    bool m_isConnected;
    std::mutex m_refMutex;
};

const SignalConnection::IdType SignalConnection::sInvalidId(0);

SignalConnection::SignalConnection(uint64_t id, DisconnectCallbackType disconnectCallback)
      : m_id(id),
        m_reference(new Reference(0)),
        m_disconnectCallback(std::move(disconnectCallback)) {
    m_reference->increase();
}

SignalConnection::SignalConnection() : m_id(0), m_reference(nullptr), m_disconnectCallback(nullptr) {}

SignalConnection::SignalConnection(const SignalConnection& other)
      : m_id(other.m_id),
        m_reference(other.m_reference),
        m_disconnectCallback(other.m_disconnectCallback) {
    if (m_reference) {
        m_reference->increase();
    }
}

SignalConnection::SignalConnection(SignalConnection&& other) noexcept
      : m_id(other.m_id),
        m_reference(other.m_reference),
        m_disconnectCallback(other.m_disconnectCallback) {
    other.m_id = sInvalidId;
    other.m_reference = nullptr;
    other.m_disconnectCallback = nullptr;
}

SignalConnection::~SignalConnection() {
    if (m_reference && m_reference->getCount() != 0) {
        if (m_reference->decrease() == 0) {
            disconnectWithCallback();
            delete m_reference;
            m_reference = nullptr;
        }
    }
}

SignalConnection& SignalConnection::operator=(const SignalConnection& other) {
    new (this) SignalConnection(other);
    return *this;
}

SignalConnection& SignalConnection::operator=(SignalConnection&& other) noexcept {
    new (this) SignalConnection(std::move(other));
    return *this;
}

SignalConnection::IdType SignalConnection::getId() const {
    return m_id;
}

void SignalConnection::disconnect() {
    disconnectWithCallback();
}

bool SignalConnection::isConnected() {
    if (m_reference) {
        return m_reference->isConnected();
    }
    return false;
}

void SignalConnection::disconnectWithCallback() {
    if (isConnected()) {
        if (m_disconnectCallback) {
            m_disconnectCallback(*this);
            m_disconnectCallback = nullptr;
        }
        disconnectWithoutCallback();
    }
}

void SignalConnection::disconnectWithoutCallback() {
    if (m_reference) {
        m_reference->disconnect();
    }
}

}  // namespace engine
