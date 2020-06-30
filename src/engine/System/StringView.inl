#include <System/StringView.hpp>

#include <Util/UTF.hpp>

#include <cstring>

namespace engine {

inline const std::size_t StringView::sInvalidPos = std::size_t(-1);

constexpr StringView::StringView(const char8* utf8String) {
    if (utf8String && utf8String[0] != 0) {
        std::size_t length = std::strlen(utf8String);
        if (length > 0) {
            if (utf::IsValidUTF<8>(utf8String, utf8String + length)) {
                m_data = utf8String;
                m_size = length;
            } else {
                ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
            }
        };
    }
}

constexpr StringView::StringView(const String& string) : m_data(string.getData()), m_size(string.getSize()) {}

constexpr StringView& StringView::operator=(const StringView& right) = default;

constexpr StringView& StringView::operator=(const String& right) {
    m_data = right.getData();
    m_size = right.getSize();
    return *this;
}

constexpr StringView& StringView::operator=(const char8* right) {
    const auto* utf8String = right;
    if (utf8String && utf8String[0] != 0) {
        std::size_t length = std::strlen(utf8String);
        if (length > 0) {
            if (utf::IsValidUTF<8>(utf8String, utf8String + length)) {
                m_data = utf8String;
                m_size = length;
            } else {
                ENGINE_THROW(std::runtime_error("invalid utf8 convertion."));
            }
        };
    }
    return *this;
}

constexpr char8 StringView::operator[](std::size_t index) const {
    return m_data[index];
}

constexpr std::size_t StringView::getSize() const {
    return utf::GetSizeUTF<8>(m_data, m_data + m_size);
}

constexpr bool StringView::isEmpty() const {
    return m_size == 0;
}

constexpr std::size_t StringView::find(const StringView& str, std::size_t start) const {
    // Iterate to the start codepoint
    const auto* startIt(m_data);
    for (std::size_t i = 0; i < start; i++) {
        startIt = utf::NextUTF<8>(startIt, m_data + m_size);
        if (startIt == m_data + m_size) {
            return sInvalidPos;
        }
    }
    // Find the string
    const auto* findIt(std::search(startIt, m_data + m_size, str.m_data, str.m_data + m_size));
    return (findIt == m_data + m_size) ? sInvalidPos : utf::GetSizeUTF<8>(m_data, findIt);
}

constexpr std::size_t StringView::findFirstOf(const StringView& str, std::size_t pos) const {
    size_t strSize = getSize();

    if (pos >= strSize) {
        return sInvalidPos;
    }

    // Iterate to the start codepoint
    const auto* startIt(m_data);
    for (std::size_t i = 0; i < pos; i++) {
        startIt = utf::NextUTF<8>(startIt, m_data + m_size);
        if (startIt == m_data + m_size) {
            return sInvalidPos;
        }
    }

    // Find one of the UTF-8 codepoints
    const auto* endIt(startIt);
    while (true) {
        if (startIt == m_data + m_size) {
            return sInvalidPos;
        }
        endIt = utf::NextUTF<8>(endIt, m_data + m_size);
        const auto* findIt(std::search(str.m_data, str.m_data + m_size, startIt, endIt));
        if (findIt != str.m_data + m_size) {
            return utf::GetSizeUTF<8>(m_data, startIt);
        }
        startIt = endIt;
    }
}

constexpr std::size_t StringView::findLastOf(const StringView& str, std::size_t pos) const {
    // Iterate to the start codepoint
    const auto* startIt(m_data);
    if (pos == sInvalidPos) {
        startIt = m_data + m_size;
    } else {
        for (std::size_t i = 0; i < pos + 1; i++) {
            startIt = utf::NextUTF<8>(startIt, m_data + m_size);
            if (startIt == m_data + m_size) {
                break;
            }
        }
    }

    // Find one of the UTF-8 codepoints
    const auto* endIt(startIt);
    while (true) {
        if (startIt == m_data) {
            return sInvalidPos;
        }
        endIt = utf::PriorUTF<8>(endIt, m_data);
        const auto* findIt(std::search(str.m_data, str.m_data + m_size, endIt, startIt));
        if (findIt != str.m_data + m_size) {
            return utf::GetSizeUTF<8>(m_data, endIt);
        }
        startIt = endIt;
    }

    return sInvalidPos;
}

constexpr StringView StringView::subString(std::size_t position, std::size_t length) const {
    // Iterate to the start codepoint
    const auto* startIt(m_data);
    for (std::size_t i = 0; i < position; i++) {
        startIt = utf::NextUTF<8>(startIt, m_data + m_size);
        if (startIt == m_data + m_size) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
    }
    // Iterate to the end codepoint
    const auto* endIt(startIt);
    for (std::size_t i = 0; i < length; i++) {
        endIt = utf::NextUTF<8>(endIt, m_data + m_size);
        if (endIt == m_data + m_size) {
            break;
        }
    }
    StringView result;
    result.m_data = startIt;
    result.m_size = endIt - startIt;
    return result;
}

constexpr const char8* StringView::getData() const {
    return m_data;
}

constexpr StringView::const_iterator StringView::begin() {
    return StringView::iterator(std::make_pair(m_data, m_data + m_size), m_data);
}

constexpr StringView::const_iterator StringView::begin() const {
    return StringView::iterator(std::make_pair(m_data, m_data + m_size), m_data);
}

constexpr StringView::const_iterator StringView::end() {
    return StringView::iterator(std::make_pair(m_data, m_data + m_size), m_data + m_size);
}

constexpr StringView::const_iterator StringView::end() const {
    return StringView::iterator(std::make_pair(m_data, m_data + m_size), m_data + m_size);
}

inline bool operator==(const StringView& left, const StringView& right) {
    return left.getSize() == right.getSize() && std::memcmp(left.getData(), right.getData(), right.getSize()) == 0;
}

inline bool operator!=(const StringView& left, const StringView& right) {
    return !(left == right);
}

inline bool operator<(const StringView& left, const StringView& right) {
    return left.getSize() < right.getSize();
}

inline bool operator>(const StringView& left, const StringView& right) {
    return right < left;
}

inline bool operator<=(const StringView& left, const StringView& right) {
    return !(right < left);
}

inline bool operator>=(const StringView& left, const StringView& right) {
    return !(left < right);
}

inline std::ostream& operator<<(std::ostream& os, const StringView& str) {
    return os.write(str.getData(), str.getSize());
}

}  // namespace engine
