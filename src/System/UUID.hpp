#pragma once

#include <Util/Prerequisites.hpp>

#include <System/String.hpp>

namespace engine {

////////////////////////////////////////////////////////////
/// \brief Class designed to generate a UUID implementing the RFC 4122
/// \details See https://tools.ietf.org/html/rfc4122
////////////////////////////////////////////////////////////
class ENGINE_API UUID {
private:
    UUID();

public:
    ////////////////////////////////////////////////////////////
    /// \brief Generate UUID version 4
    /// \details See https://tools.ietf.org/html/rfc4122#section-4.4
    ////////////////////////////////////////////////////////////
    static UUID UUID4();

    ////////////////////////////////////////////////////////////
    /// \brief Implicit conversion to a String
    /// \details See https://tools.ietf.org/html/rfc4122#section-3 to understand
    ///          how the UUID string representation should be formed
    ////////////////////////////////////////////////////////////
    operator String();

private:
    struct {
        uint32 time_low;
        uint16 time_mid;
        uint16 time_hi_and_version;
        uint8 clock_seq_hi_and_reserved;
        uint8 clock_seq_low;
        byte node[6];
    } m_uuid;
};

}  // namespace engine
