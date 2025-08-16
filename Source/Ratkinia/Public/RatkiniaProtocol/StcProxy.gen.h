// 2025. 08. 16. 23:45. Ratkinia Protocol Generator에 의해 생성됨.

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

        void CreateCharacterResponse(const CreateCharacterResponse_CreateCharacterResult Successful)
        {
            class CreateCharacterResponse CreateCharacterResponseMessage;
            CreateCharacterResponseMessage.set_successful(Successful);
            static_cast<TDerivedProxy*>(this)->WriteMessage(StcMessageType::CreateCharacterResponse, CreateCharacterResponseMessage);
        }
    };
}

#endif