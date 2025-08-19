//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_CTSSTUB_GEN_H
#define RATKINIAPROTOCOL_CTSSTUB_GEN_H

#include "CtsMessageType.gen.h"
#include "Cts.pb.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedStub>
    class TCtsStub
    {
    public:
        virtual ~TCtsStub() = default;

        virtual void OnUnknownMessageType(CtsMessageType MessageType) = 0;

        virtual void OnParseMessageFailed(CtsMessageType MessageType) = 0;

        virtual void OnUnhandledMessageType(CtsMessageType MessageType) = 0;

        virtual void OnLoginRequest(FString Account, FString Password) { static_cast<TDerivedStub*>(this)->OnUnhandledMessageType(CtsMessageType::LoginRequest); }

        virtual void OnRegisterRequest(FString Account, FString Password) { static_cast<TDerivedStub*>(this)->OnUnhandledMessageType(CtsMessageType::RegisterRequest); }

        virtual void OnCreateCharacter(FString Name) { static_cast<TDerivedStub*>(this)->OnUnhandledMessageType(CtsMessageType::CreateCharacter); }

        virtual void OnLoadMyCharacters() { static_cast<TDerivedStub*>(this)->OnUnhandledMessageType(CtsMessageType::LoadMyCharacters); }

        void HandleCts(const uint16 MessageType, const uint16 BodySize, const char* const Body)
        {
            switch (static_cast<int32_t>(MessageType))
            {
                case static_cast<int32_t>(CtsMessageType::LoginRequest):
                {
                    LoginRequest LoginRequestMessage;
                    if (!LoginRequestMessage.ParseFromArray(Body, BodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(static_cast<CtsMessageType>(MessageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnLoginRequest(FString{UTF8_TO_TCHAR(LoginRequestMessage.account().c_str())}, FString{UTF8_TO_TCHAR(LoginRequestMessage.password().c_str())});
                    return;
                }
                case static_cast<int32_t>(CtsMessageType::RegisterRequest):
                {
                    RegisterRequest RegisterRequestMessage;
                    if (!RegisterRequestMessage.ParseFromArray(Body, BodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(static_cast<CtsMessageType>(MessageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnRegisterRequest(FString{UTF8_TO_TCHAR(RegisterRequestMessage.account().c_str())}, FString{UTF8_TO_TCHAR(RegisterRequestMessage.password().c_str())});
                    return;
                }
                case static_cast<int32_t>(CtsMessageType::CreateCharacter):
                {
                    CreateCharacter CreateCharacterMessage;
                    if (!CreateCharacterMessage.ParseFromArray(Body, BodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(static_cast<CtsMessageType>(MessageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnCreateCharacter(FString{UTF8_TO_TCHAR(CreateCharacterMessage.name().c_str())});
                    return;
                }
                case static_cast<int32_t>(CtsMessageType::LoadMyCharacters):
                {
                    LoadMyCharacters LoadMyCharactersMessage;
                    if (!LoadMyCharactersMessage.ParseFromArray(Body, BodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(static_cast<CtsMessageType>(MessageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnLoadMyCharacters();
                    return;
                }
                default:
                {
                    static_cast<TDerivedStub*>(this)->OnUnknownMessageType(static_cast<CtsMessageType>(MessageType));
                    return;
                }
            }

        }
    };
}
#endif