// 2025. 08. 16. 23:45. Ratkinia Protocol Generator에 의해 생성됨.

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
        void LoginRequest(const FString& Account, const FString& Password)
        {
            class LoginRequest LoginRequestMessage;
            LoginRequestMessage.set_account(std::string{TCHAR_TO_UTF8(*Account)});
            LoginRequestMessage.set_password(std::string{TCHAR_TO_UTF8(*Password)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(CtsMessageType::LoginRequest, LoginRequestMessage);
        }

        void RegisterRequest(const FString& Account, const FString& Password)
        {
            class RegisterRequest RegisterRequestMessage;
            RegisterRequestMessage.set_account(std::string{TCHAR_TO_UTF8(*Account)});
            RegisterRequestMessage.set_password(std::string{TCHAR_TO_UTF8(*Password)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(CtsMessageType::RegisterRequest, RegisterRequestMessage);
        }

        void CreateCharacter(const FString& Name)
        {
            class CreateCharacter CreateCharacterMessage;
            CreateCharacterMessage.set_name(std::string{TCHAR_TO_UTF8(*Name)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(CtsMessageType::CreateCharacter, CreateCharacterMessage);
        }
    };
}

#endif