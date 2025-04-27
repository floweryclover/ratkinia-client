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


bool URatkiniaClientSubsystem::Connect(const FString& ServerAddress, int32_t ServerPort)
{
	if (NetworkWorker.IsValid())
	{
		return false;
	}

	NetworkWorker = MakeUnique<FNetworkWorker>(RatkiniaProtocol::MessageMaxSize * 128);
	NetworkWorker->Connect(ServerAddress, ServerPort);

	return true;
}

void URatkiniaClientSubsystem::Disconnect()
{
	NetworkWorker.Reset();
}
