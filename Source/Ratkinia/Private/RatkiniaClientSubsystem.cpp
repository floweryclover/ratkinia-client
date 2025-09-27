// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaClientSubsystem.h"
#include "NetworkWorker.h"
#include <WinSock2.h>

#include "Sockets.h"

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

const FString& URatkiniaClientSubsystem::GetDisconnectedReason() const
{
	if (!NetworkWorker.IsValid())
	{
		static const FString EmptyReason;
		return EmptyReason;
	}
	return NetworkWorker->GetDisconnectedReason();
}

void URatkiniaClientSubsystem::ClearSession(const FString& Reason)
{
	if (!NetworkWorker)
	{
		return;
	}
	
	NetworkWorker->Disconnect(Reason);
	NetworkWorker.Reset();
}
