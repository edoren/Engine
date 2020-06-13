#include <System/SignalConnection.hpp>

namespace engine {

class SignalConnection::Reference {
public:
    Reference(uint64_t count) : m_count(count), is_connected(true) {}

    uint64_t Increase() {
        std::lock_guard<std::mutex> lock(m_ref_mutex);
        return ++m_count;
    }

    uint64_t Decrease() {
        std::lock_guard<std::mutex> lock(m_ref_mutex);
        return --m_count;
    }

    uint64_t GetCount() const {
        return m_count;
    }

    void Disconnect() {
        if (IsConnected()) {
            std::lock_guard<std::mutex> lock(m_ref_mutex);
            is_connected = false;
        }
    }

    bool IsConnected() const {
        return is_connected;
    }

private:
    uint64_t m_count;
    bool is_connected;
    std::mutex m_ref_mutex;
};

const SignalConnection::IdType SignalConnection::InvalidId(0);

SignalConnection::SignalConnection(uint64_t id, DisconnectCallbackType disconnect_callback)
      : m_id(id),
        m_reference(new Reference(0)),
        m_disconnect_callback(disconnect_callback) {
    m_reference->Increase();
}

SignalConnection::SignalConnection() : m_id(0), m_reference(nullptr), m_disconnect_callback(nullptr) {}

SignalConnection::SignalConnection(const SignalConnection& other)
      : m_id(other.m_id),
        m_reference(other.m_reference),
        m_disconnect_callback(other.m_disconnect_callback) {
    if (m_reference) {
        m_reference->Increase();
    }
}

SignalConnection::SignalConnection(SignalConnection&& other)
      : m_id(other.m_id),
        m_reference(other.m_reference),
        m_disconnect_callback(other.m_disconnect_callback) {
    other.m_id = InvalidId;
    other.m_reference = nullptr;
    other.m_disconnect_callback = nullptr;
}

SignalConnection::~SignalConnection() {
    if (m_reference && m_reference->GetCount() != 0) {
        if (m_reference->Decrease() == 0) {
            DisconnectWithCallback();
            delete m_reference;
            m_reference = nullptr;
        }
    }
}

SignalConnection& SignalConnection::operator=(const SignalConnection& other) {
    new (this) SignalConnection(other);
    return *this;
}

SignalConnection& SignalConnection::operator=(SignalConnection&& other) {
    new (this) SignalConnection(std::move(other));
    return *this;
}

SignalConnection::IdType SignalConnection::GetId() const {
    return m_id;
}

void SignalConnection::Disconnect() {
    DisconnectWithCallback();
}

bool SignalConnection::IsConnected() {
    if (m_reference) {
        return m_reference->IsConnected();
    }
    return false;
}

void SignalConnection::DisconnectWithCallback() {
    if (IsConnected()) {
        if (m_disconnect_callback) {
            m_disconnect_callback(*this);
            m_disconnect_callback = nullptr;
        }
        DisconnectWithoutCallback();
    }
}

void SignalConnection::DisconnectWithoutCallback() {
    if (m_reference) {
        m_reference->Disconnect();
    }
}

}  // namespace engine
