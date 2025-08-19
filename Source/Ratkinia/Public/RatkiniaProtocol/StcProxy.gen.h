//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_STCPROXY_GEN_H
#define RATKINIAPROTOCOL_STCPROXY_GEN_H

#include "Stc.pb.h"
#include "StcMessageType.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class TStcProxy
    {
    public:
        void Disconnect(const FString& Detail)
        {
            class Disconnect DisconnectMessage;
            DisconnectMessage.set_detail(std::string{TCHAR_TO_UTF8(*Detail)});
            static_cast<TDerivedProxy*>(this)->WriteMessage(StcMessageType::Disconnect, DisconnectMessage);
        }

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

        void CreateCharacterResponse(const CreateCharacterResponse_CreateCharacterResult Result)
        {
            class CreateCharacterResponse CreateCharacterResponseMessage;
            CreateCharacterResponseMessage.set_result(Result);
            static_cast<TDerivedProxy*>(this)->WriteMessage(StcMessageType::CreateCharacterResponse, CreateCharacterResponseMessage);
        }

        void SendMyCharacters(auto&& OriginalCharacterLoadDatasRange, auto&& CharacterLoadDatasSetter)
        {
            class SendMyCharacters SendMyCharactersMessage;
            for (auto&& OriginalCharacterLoadDatasElement : OriginalCharacterLoadDatasRange)
            {
                SendMyCharacters_CharacterLoadData* const NewCharacterLoadDatasElement = SendMyCharactersMessage.add_character_load_datas();
                CharacterLoadDatasSetter(std::forward<decltype(OriginalCharacterLoadDatasElement)>(OriginalCharacterLoadDatasElement), *NewCharacterLoadDatasElement);
            }
            static_cast<TDerivedProxy*>(this)->WriteMessage(StcMessageType::SendMyCharacters, SendMyCharactersMessage);
        }
    };
}

#endif