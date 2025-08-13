// Auto-generated from Ratkinia Protocol Generator.

#ifndef RATKINIA_PROTOCOL_H
#define RATKINIA_PROTOCOL_H

#include <cstdint>

namespace RatkiniaProtocol
{
    struct MessageHeader final
    {
        uint16 MessageType;
        uint16 BodySize;
    };

    constexpr SIZE_T MessageMaxSize = 1024 + sizeof(MessageHeader);
    constexpr SIZE_T MessageHeaderSize = sizeof(MessageHeader);

    enum class CtsMessageType : uint16_t
    {
        LoginRequest = 0,
        RegisterRequest = 1,
    };

    enum class StcMessageType : uint16_t
    {
        LoginResponse = 0,
        RegisterResponse = 1,
    };
}

#endif