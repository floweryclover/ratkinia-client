//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_H
#define RATKINIA_PROTOCOL_H

#include <cstdint>

namespace RatkiniaProtocol
{
#pragma pack(push, 1)
    struct FMessageHeader final
    {
        uint16 MessageType;
        uint16 BodySize;
    };
#pragma pack(pop)

    constexpr SIZE_T MessageMaxSize = 1024 + sizeof(FMessageHeader);
    constexpr SIZE_T MessageHeaderSize = sizeof(FMessageHeader);
    constexpr const char* const Version = "20250819.211618";
}

#endif