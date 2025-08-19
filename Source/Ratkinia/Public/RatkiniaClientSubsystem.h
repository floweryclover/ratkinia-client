// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CtsProxy.gen.h"
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
                                                    public RatkiniaProtocol::TCtsProxy<URatkiniaClientSubsystem>
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
	void WriteMessage(const RatkiniaProtocol::CtsMessageType MessageType, const TMessage& Message)
	{
		if (!NetworkWorker)
		{
			return;
		}
		
		NetworkWorker->Send(Message, MessageType);
	}

	TOptional<FMessagePeekResult> TryPeekMessage()
	{
		if (!NetworkWorker)
		{
			return NullOpt;
		}
		
		return NetworkWorker->TryPeekMessage();
	}

	void PopMessage(const FMessagePeekResult& PeekResult)
	{
		if (!NetworkWorker)
		{
			return;
		}
		
		NetworkWorker->Pop(PeekResult);
	}

	ERatkiniaConnectionState GetConnectionState() const
	{
		if (!NetworkWorker.IsValid())
		{
			return ERatkiniaConnectionState::NotConnected;
		}

		return NetworkWorker->GetConnectionState();
	}

	const FString& GetDisconnectedReason() const;

private:
	FString DisconnectedReason;
	TUniquePtr<FNetworkWorker> NetworkWorker;
};
