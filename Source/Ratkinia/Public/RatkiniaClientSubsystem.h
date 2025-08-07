// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CtsProxy.gen.h"
#include "ScopedNetworkMessage.h"
#include "NetworkWorker.h"

#include "CoreMinimal.h"
#include "RatkiniaClientSubsystem.generated.h"

class FNetworkWorker;

UENUM()
enum class EMessagePopResult : uint8
{
	Pop,
	Empty,
	Disconnected
};

/**
 * 
 */
UCLASS()
class RATKINIA_API URatkiniaClientSubsystem final : public UGameInstanceSubsystem,
                                                    public RatkiniaProtocol::CtsProxy<URatkiniaClientSubsystem>
{
	GENERATED_BODY()

public:
	explicit URatkiniaClientSubsystem();

	explicit URatkiniaClientSubsystem(FVTableHelper& Helper);

	virtual ~URatkiniaClientSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	void Connect(const FString& ServerAddress, int32_t ServerPort);
	
	void ClearSession();
	
	template <typename TMessage>
	void WriteMessage(const uint64_t, const RatkiniaProtocol::CtsMessageType MessageType, const TMessage& Message)
	{
		if (!NetworkWorker.IsValid())
		{
			return;
		}

		NetworkWorker->Send(Message, MessageType);
	}

	FORCEINLINE TOptional<TScopedNetworkMessage<FNetworkWorker>> TryPopMessage()
	{
		if (!NetworkWorker.IsValid())
		{
			return NullOpt;
		}

		return NetworkWorker->TryPopMessage();
	}

	FORCEINLINE ERatkiniaConnectionState GetConnectionState() const
	{
		if (!NetworkWorker.IsValid())
		{
			return ERatkiniaConnectionState::NotConnected;
		}

		return NetworkWorker->GetConnectionState();
	}

	FString GetDisconnectedReason() const;

private:
	FString DisconnectedReason;
	TUniquePtr<FNetworkWorker> NetworkWorker;
};
