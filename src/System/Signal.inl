namespace engine {

template <typename... Args>
Signal<Args...>::Signal() : m_current_id(0) {}

template <typename... Args>
Signal<Args...>::~Signal() {
    DisconnectAll();
}

// Connects a member function to this Signal
template <typename... Args>
template <typename T>
uint32 Signal<Args...>::Connect(T* inst, void (T::*func)(Args...)) {
    return Connect([=](Args... args) -> void { (inst->*func)(args...); });
}

// Connects a const member function to this Signal
template <typename... Args>
template <typename T>
uint32 Signal<Args...>::Connect(T* inst, void (T::*func)(Args...) const) {
    return Connect([=](Args... args) -> void { (inst->*func)(args...); });
}

// Connects a std::function to the signal. The returned
// value can be used to disconnect the function again
template <typename... Args>
uint32 Signal<Args...>::Connect(const Slot<Args...>& slot) {
    m_slots.insert(std::make_pair(++m_current_id, slot));
    return m_current_id;
}

// Disconnects a previously connected function
template <typename... Args>
void Signal<Args...>::Disconnect(uint32 id) {
    m_slots.erase(id);
}

// Disconnects all previously connected functions
template <typename... Args>
void Signal<Args...>::DisconnectAll() {
    m_slots.clear();
}

// Calls all connected functions
template <typename... Args>
void Signal<Args...>::Emit(Args... args) {
    for (auto it : m_slots) {
        it.second(args...);
    }
}

}  // namespace engine
