// Auto-generated from Ratkinia Protocol Generator.

#ifndef STCSTUB_GEN_H
#define STCSTUB_GEN_H

#include "RatkiniaProtocol.gen.h"
#include "Stc.pb.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedStub>
    class StcStub
    {
    public:
        virtual ~StcStub() = default;

        virtual void OnUnknownMessageType(StcMessageType MessageType) = 0;

        virtual void OnParseMessageFailed(StcMessageType MessageType) = 0;

        virtual void OnUnhandledMessageType(StcMessageType MessageType) = 0;

        virtual void OnLoginResponse(const LoginResponse_LoginResult Result) { static_cast<TDerivedStub*>(this)->OnUnhandledMessageType(StcMessageType::LoginResponse); }

        virtual void OnRegisterResponse(const bool bSuccessful, FString FailedReason) { static_cast<TDerivedStub*>(this)->OnUnhandledMessageType(StcMessageType::RegisterResponse); }

        void HandleStc(
            const uint16 MessageType,
            const uint16 BodySize,
            const char* const Body)
        {
            switch (static_cast<int32_t>(MessageType))
            {
                case static_cast<int32_t>(StcMessageType::LoginResponse):
                {
                    LoginResponse LoginResponseMessage;
                    if (!LoginResponseMessage.ParseFromArray(Body, BodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(static_cast<StcMessageType>(MessageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnLoginResponse(LoginResponseMessage.result());
                    return;
                }
                case static_cast<int32_t>(StcMessageType::RegisterResponse):
                {
                    RegisterResponse RegisterResponseMessage;
                    if (!RegisterResponseMessage.ParseFromArray(Body, BodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(static_cast<StcMessageType>(MessageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnRegisterResponse(RegisterResponseMessage.successful(), FString{UTF8_TO_TCHAR(RegisterResponseMessage.failed_reason().c_str())});
                    return;
                }
                default:
                {
                    static_cast<TDerivedStub*>(this)->OnUnknownMessageType(static_cast<StcMessageType>(MessageType));
                    return;
                }
            }

        }
    };
}
#endif