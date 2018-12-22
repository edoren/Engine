
#include <System/UUID.hpp>

namespace {

const char sHexdigits[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

}  // namespace

namespace engine {

static_assert(sizeof(UUID) == 16, "UUID class size should be 16 bytes");

UUID::UUID() {}

UUID UUID::UUID4() {
    static std::random_device rd;
    static std::mt19937_64 rng(rd());
    static std::uniform_int_distribution<uint64> dist;

    UUID output;

    uint64* uint64_uuid = reinterpret_cast<uint64*>(&output.m_uuid);
    for (int i = 0; i < 2; i++) {
        uint64_uuid[i] = dist(rng);
    }

    output.m_uuid.clock_seq_hi_and_reserved =
        (output.m_uuid.clock_seq_hi_and_reserved & 0x3f) | 0x80;
    output.m_uuid.clock_seq_low = (output.m_uuid.clock_seq_low & 0x0f) | 0x40;

    return output;
}

UUID::operator String() {
    std::string output;
    output.reserve(36);

    byte* byte_uuid = reinterpret_cast<byte*>(&m_uuid);
    for (int i = 0; i < 16; i++) {
        if ((i == 4) | (i == 6) | (i == 8) | (i == 10)) output.push_back('-');
        output.push_back(sHexdigits[(byte_uuid[i] & 0xf0) >> 4]);
        output.push_back(sHexdigits[byte_uuid[i] & 0x0f]);
    }

    return String(std::move(output));
}

}  // namespace engine