// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaClientSubsystem.h"
#include "NetworkWorker.h"

URatkiniaClientSubsystem::URatkiniaClientSubsystem() = default;

URatkiniaClientSubsystem::URatkiniaClientSubsystem(FVTableHelper& Helper)
	: UGameInstanceSubsystem{Helper}
{
}

URatkiniaClientSubsystem::~URatkiniaClientSubsystem() = default;

void URatkiniaClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WSADATA WsaData{};
	check(WSAStartup(MAKEWORD(2, 2), &WsaData) == 0);
}

void URatkiniaClientSubsystem::Deinitialize()
{
	Super::Deinitialize();
	WSACleanup();
}


void URatkiniaClientSubsystem::Connect(const FString& ServerAddress, int32_t ServerPort,
                                       const TFunction<void(const FString&)>& OnFailure)
{
	if (NetworkWorker.IsValid())
	{
		OnFailure(TEXT("이미 서버에 접속되어 있거나 접속 중입니다."));
		return;
	}

	NetworkWorker = MakeUnique<FNetworkWorker>(RatkiniaProtocol::MessageMaxSize * 128, ServerAddress, ServerPort);
}

void URatkiniaClientSubsystem::Disconnect()
{
	NetworkWorker->Stop();
}
