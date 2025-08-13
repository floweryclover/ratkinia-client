// Auto-generated from all.desc.

#ifndef CTSPROXY_GEN_H
#define CTSPROXY_GEN_H

#include "Cts.pb.h"
#include "RatkiniaProtocol.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class CtsProxy
    {
    public:
        void LoginRequest(const FString& Id, const FString& Password)
        {
            class LoginRequest LoginRequestMessage;
            LoginRequestMessage.set_id(std::string{TCHAR_TO_UTF8(*Id)});
            LoginRequestMessage.set_password(std::string{TCHAR_TO_UTF8(*Password)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(CtsMessageType::LoginRequest, LoginRequestMessage);
        }

        void RegisterRequest(const FString& Id, const FString& Password)
        {
            class RegisterRequest RegisterRequestMessage;
            RegisterRequestMessage.set_id(std::string{TCHAR_TO_UTF8(*Id)});
            RegisterRequestMessage.set_password(std::string{TCHAR_TO_UTF8(*Password)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(CtsMessageType::RegisterRequest, RegisterRequestMessage);
        }
    };
}

#endif