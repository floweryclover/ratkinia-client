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

void URatkiniaClientSubsystem::Connect(const FString& ServerAddress, const int32_t ServerPort)
{
	NetworkWorker = MakeUnique<FNetworkWorker>(ServerAddress, ServerPort);
}

FString URatkiniaClientSubsystem::GetDisconnectedReason() const
{
	if (!NetworkWorker.IsValid())
	{
		return {};
	}
	
	return UTF8_TO_TCHAR(NetworkWorker->GetDisconnectedReason().c_str());
}

void URatkiniaClientSubsystem::ClearSession()
{
	if (!NetworkWorker)
	{
		return;
	}
	
	NetworkWorker->Disconnect("사용자가 연결을 종료하였습니다.");
	NetworkWorker.Reset();
}
