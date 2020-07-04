#include <System/StringView.hpp>

#include <Util/UTF.hpp>

#include <cstring>

namespace engine {

inline const StringView::size_type StringView::sInvalidPos = size_type(-1);

constexpr StringView::StringView(const char8* utf8String) {
    if (utf8String && utf8String[0] != 0) {
        size_type length = std::strlen(utf8String);
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
        size_type length = std::strlen(utf8String);
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

constexpr char8 StringView::operator[](size_type index) const {
    return m_data[index];
}

constexpr StringView::size_type StringView::getSize() const {
    return utf::GetSizeUTF<8>(m_data, m_data + m_size);
}

constexpr bool StringView::isEmpty() const {
    return m_size == 0;
}

constexpr StringView::size_type StringView::find(const StringView& str, size_type start) const {
    // Iterate to the start codepoint
    const auto* startIt(m_data);
    for (size_type i = 0; i < start; i++) {
        startIt = utf::NextUTF<8>(startIt, m_data + m_size);
        if (startIt == m_data + m_size) {
            return sInvalidPos;
        }
    }
    // Find the string
    const auto* findIt(std::search(startIt, m_data + m_size, str.m_data, str.m_data + m_size));
    return (findIt == m_data + m_size) ? sInvalidPos : utf::GetSizeUTF<8>(m_data, findIt);
}

constexpr StringView::size_type StringView::findFirstOf(const StringView& str, size_type pos) const {
    size_t strSize = getSize();

    if (pos >= strSize) {
        return sInvalidPos;
    }

    // Iterate to the start codepoint
    const auto* startIt(m_data);
    for (size_type i = 0; i < pos; i++) {
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

constexpr StringView::size_type StringView::findLastOf(const StringView& str, size_type pos) const {
    // Iterate to the start codepoint
    const auto* startIt(m_data);
    if (pos == sInvalidPos) {
        startIt = m_data + m_size;
    } else {
        for (size_type i = 0; i < pos + 1; i++) {
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

constexpr StringView StringView::subString(size_type position, size_type length) const {
    // Iterate to the start codepoint
    const auto* startIt(m_data);
    for (size_type i = 0; i < position; i++) {
        startIt = utf::NextUTF<8>(startIt, m_data + m_size);
        if (startIt == m_data + m_size) {
            ENGINE_THROW(std::out_of_range("the specified position is out of the string range"));
        }
    }
    // Iterate to the end codepoint
    const auto* endIt(startIt);
    for (size_type i = 0; i < length; i++) {
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

constexpr StringView::const_iterator StringView::cbegin() const {
    return StringView::iterator(std::make_pair(m_data, m_data + m_size), m_data);
}

constexpr StringView::const_iterator StringView::end() {
    return StringView::iterator(std::make_pair(m_data, m_data + m_size), m_data + m_size);
}

constexpr StringView::const_iterator StringView::cend() const {
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
