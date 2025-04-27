// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "LoginGameMode.h"
#include "RatkiniaProtocol.h"
#include "STC_Auth.pb.h"
#include "RatkiniaClientSubsystem.h"
#include "Ratkinia/Ratkinia.h"

ALoginGameMode::ALoginGameMode()
	: MessageBodyBuffer{MakeUnique<char[]>(RatkiniaProtocol::MessageMaxSize)},
	  bIsConnected{false}
{
}

void ALoginGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URatkiniaClientSubsystem* const RatkiniaClient = GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>();
	while (true)
	{
		uint16 MessageType;
		uint16 MessageBodyLength;
		const EMessagePopResult PopResult = RatkiniaClient->PopMessage(MessageType, MessageBodyLength,
		                                                               MessageBodyBuffer.Get(),
		                                                               RatkiniaProtocol::MessageMaxSize);
		if (PopResult != EMessagePopResult::Pop)
		{
			break;
		}

		if (static_cast<RatkiniaProtocol::Stc::MessageType>(MessageType) == RatkiniaProtocol::Stc::MessageType::LoginResponse)
		{
			RatkiniaProtocol::Stc::LoginResponse LoginResponse;
			LoginResponse.ParseFromArray(MessageBodyBuffer.Get(), MessageBodyLength);

			if (LoginResponse.successful())
			{
				UE_LOG(LogRatkinia, Log, TEXT("로그인 성공"));
			}
			else
			{
				UE_LOG(LogRatkinia, Log, TEXT("로그인 실패: %hs"), UTF8_TO_TCHAR(LoginResponse.failure_reason().c_str()));
			}
		}
	}
}
