namespace engine {

template <typename... Args>
Signal<Args...>::Signal() : m_current_id(0) {}

template <typename... Args>
Signal<Args...>::Signal(const Signal<Args...>& other) : m_current_id(0) {
    std::lock_guard<std::mutex> lock(other.m_slots_mutex);
    for (auto& pair : other.m_slots) {
        SlotType slot = pair.second.second;
        Connect(slot);
    }
}

template <typename... Args>
Signal<Args...>::Signal(Signal<Args...>&& other) : m_current_id(other.m_current_id) {
    std::lock_guard<std::mutex> lock(other.m_slots_mutex);
    m_slots = std::move(other.m_slots);
}

template <typename... Args>
Signal<Args...>::~Signal() {
    DisconnectAll();
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
SignalConnection& Signal<Args...>::Connect(InstanceType& instance, void (MFInstanceType::*func)(Args...)) {
    static_assert(!std::is_pointer<InstanceType>::value, "Instance cannot be a pointer, it must be a reference");
    static_assert(std::is_member_function_pointer<decltype(func)>::value,
                  "Function must be a non-static member function pointer");
    static_assert(std::is_base_of<MFInstanceType, InstanceType>::value, "Instance is not base of member function");
    return Connect([&instance, func](Args... args) { (instance.*func)(std::forward<Args>(args)...); });
}

template <typename... Args>
template <typename InstanceType, typename MFInstanceType>
SignalConnection& Signal<Args...>::Connect(InstanceType& instance, void (MFInstanceType::*func)(Args...) const) {
    static_assert(!std::is_pointer<InstanceType>::value, "Instance cannot be a pointer, it must be a reference");
    static_assert(std::is_member_function_pointer<decltype(func)>::value,
                  "Function must be a non-static member function pointer");
    static_assert(std::is_base_of<MFInstanceType, InstanceType>::value, "Instance is not base of member function");
    return Connect([&instance, func](Args... args) { (instance.*func)(std::forward<Args>(args)...); });
}

template <typename... Args>
SignalConnection& Signal<Args...>::Connect(const typename Signal<Args...>::SlotType& slot) {
    std::lock_guard<std::mutex> lock(m_slots_mutex);
    SignalConnection::IdType id = ++m_current_id;
    SignalConnection connection(id, [this](SignalConnection& connection) { this->Disconnect(connection); });
    auto pair = m_slots.emplace(id, std::make_pair(std::move(connection), slot));
    return pair.first->second.first;
}

template <typename... Args>
void Signal<Args...>::Disconnect(SignalConnection& connection) {
    std::lock_guard<std::mutex> lock(m_slots_mutex);
    connection.DisconnectWithoutCallback();
    m_slots.erase(connection.GetId());
}

template <typename... Args>
void Signal<Args...>::DisconnectAll() {
    std::lock_guard<std::mutex> lock(m_slots_mutex);
    for (auto& pair : m_slots) {
        pair.second.first.DisconnectWithoutCallback();
    }
    m_slots.clear();
}

template <typename... Args>
void Signal<Args...>::Emit(Args... args) {
    std::lock_guard<std::mutex> lock(m_slots_mutex);
    for (auto& pair : m_slots) {
        pair.second.second(std::forward<Args>(args)...);
    }
}

}  // namespace engine
