// Auto-generated from all.desc.

#ifndef STCPROXY_GEN_H
#define STCPROXY_GEN_H

#include "Stc.pb.h"
#include "RatkiniaProtocol.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class StcProxy
    {
    public:
        void LoginResponse(const LoginResponse_LoginResult Result)
        {
            class LoginResponse LoginResponseMessage;
            LoginResponseMessage.set_result(Result);
            static_cast<TDerivedProxy*>(this)->WriteMessage(StcMessageType::LoginResponse, LoginResponseMessage);
        }

        void RegisterResponse(const bool bSuccessful, const FString& FailedReason)
        {
            class RegisterResponse RegisterResponseMessage;
            RegisterResponseMessage.set_successful(bSuccessful);
            RegisterResponseMessage.set_failed_reason(std::string{TCHAR_TO_UTF8(*FailedReason)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(StcMessageType::RegisterResponse, RegisterResponseMessage);
        }
    };
}

#endif