#include <System/StringView.hpp>

#include <System/String.hpp>

namespace engine {

StringView::StringView(const String& string) : m_data(string.getData()), m_size(string.getDataSize()) {}

StringView& StringView::operator=(const String& right) {
    m_data = right.getData();
    m_size = right.getDataSize();
    return *this;
}

}  // namespace engine
