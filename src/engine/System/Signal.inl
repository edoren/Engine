namespace engine {

template <typename... Args>
Signal<Args...>::Signal() : m_currentId(0) {}

template <typename... Args>
Signal<Args...>::Signal(const Signal<Args...>& other) : m_currentId(0) {
    std::lock_guard<std::mutex> lock(other.m_slotsMutex);
    for (auto& pair : other.m_slots) {
        SlotType slot = pair.second.second;
        connect(slot);
    }
}

template <typename... Args>
Signal<Args...>::Signal(Signal<Args...>&& other) : m_currentId(other.m_currentId) {
    std::lock_guard<std::mutex> lock(other.m_slotsMutex);
    m_slots = std::move(other.m_slots);
}

template <typename... Args>
Signal<Args...>::~Signal() {
    disconnectAll();
}

template <typename... Args>
Signal<Args...>& Signal<Args...>::operator=(const Signal<Args...>& other) {
    new (this) Signal<Args...>(other);
    return *this;
}

template <typename... Args>
Signal<Args...>& Signal<Args...>::operator=(Signal<Args...>&& other) {
    new (this) Signal<Args...>(std::move(other));
    return *this;
}

template <typename... Args>
template <typename InstanceType, typename MFInstanceType>
SignalConnection& Signal<Args...>::connect(InstanceType& instance, void (MFInstanceType::*func)(Args...)) {
    static_assert(!std::is_pointer<InstanceType>::value, "Instance cannot be a pointer, it must be a reference");
    static_assert(std::is_member_function_pointer<decltype(func)>::value,
                  "Function must be a non-static member function pointer");
    static_assert(std::is_base_of<MFInstanceType, InstanceType>::value, "Instance is not base of member function");
    return connect([&instance, func](Args... args) { (instance.*func)(std::forward<Args>(args)...); });
}

template <typename... Args>
template <typename InstanceType, typename MFInstanceType>
SignalConnection& Signal<Args...>::connect(InstanceType& instance, void (MFInstanceType::*func)(Args...) const) {
    static_assert(!std::is_pointer<InstanceType>::value, "Instance cannot be a pointer, it must be a reference");
    static_assert(std::is_member_function_pointer<decltype(func)>::value,
                  "Function must be a non-static member function pointer");
    static_assert(std::is_base_of<MFInstanceType, InstanceType>::value, "Instance is not base of member function");
    return connect([&instance, func](Args... args) { (instance.*func)(std::forward<Args>(args)...); });
}

template <typename... Args>
SignalConnection& Signal<Args...>::connect(const typename Signal<Args...>::SlotType& slot) {
    std::lock_guard<std::mutex> lock(m_slotsMutex);
    SignalConnection::IdType id = ++m_currentId;
    SignalConnection connection(id, [this](SignalConnection& connection) { this->disconnect(connection); });
    auto pair = m_slots.emplace(id, std::make_pair(std::move(connection), slot));
    return pair.first->second.first;
}

template <typename... Args>
void Signal<Args...>::disconnect(SignalConnection& connection) {
    std::lock_guard<std::mutex> lock(m_slotsMutex);
    connection.disconnectWithoutCallback();
    m_slots.erase(connection.getId());
}

template <typename... Args>
void Signal<Args...>::disconnectAll() {
    std::lock_guard<std::mutex> lock(m_slotsMutex);
    for (auto& pair : m_slots) {
        pair.second.first.disconnectWithoutCallback();
    }
    m_slots.clear();
}

template <typename... Args>
void Signal<Args...>::emit(Args... args) {
    std::lock_guard<std::mutex> lock(m_slotsMutex);
    for (auto& pair : m_slots) {
        pair.second.second(std::forward<Args>(args)...);
    }
}

}  // namespace engine
