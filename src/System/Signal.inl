namespace engine {

template <typename... Args>
Signal<Args...>::Signal() : m_current_id(0) {}

template <typename... Args>
Signal<Args...>::~Signal() {
    DisconnectAll();
}

template <typename... Args>
template <typename T>
uint32 Signal<Args...>::Connect(T* inst, void (T::*func)(Args...)) {
    return Connect([=](Args... args) -> void { (inst->*func)(args...); });
}

template <typename... Args>
template <typename T>
uint32 Signal<Args...>::Connect(T* inst, void (T::*func)(Args...) const) {
    return Connect([=](Args... args) -> void { (inst->*func)(args...); });
}

template <typename... Args>
uint32 Signal<Args...>::Connect(const Slot<Args...>& slot) {
    m_slots.insert(std::make_pair(++m_current_id, slot));
    return m_current_id;
}

template <typename... Args>
void Signal<Args...>::Disconnect(uint32 id) {
    m_slots.erase(id);
}

template <typename... Args>
void Signal<Args...>::DisconnectAll() {
    m_slots.clear();
}

template <typename... Args>
void Signal<Args...>::Emit(Args... args) {
    for (auto& it : m_slots) {
        it.second(args...);
    }
}

}  // namespace engine
