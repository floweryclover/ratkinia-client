// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkWorker.h"
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
class RATKINIA_API URatkiniaClientSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	explicit URatkiniaClientSubsystem();

	explicit URatkiniaClientSubsystem(FVTableHelper& Helper);

	virtual ~URatkiniaClientSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	void Connect(const FString& ServerAddress, int32_t ServerPort, const TFunction<void(const FString&)>& OnFailure);

	void Disconnect();

	FORCEINLINE EMessagePopResult PopMessage(uint16& OutMessageType,
	                             uint16& OutMessageBodyLength,
	                             char* const OutMessageBodyBuffer,
	                             const int32 InMessageBodyBufferSize)
	{
		if (!NetworkWorker.IsValid())
		{
			return EMessagePopResult::Empty;
		}

		if (NetworkWorker->IsStopped())
		{
			DisconnectedReason = MoveTemp(NetworkWorker->GetEndReason());
			NetworkWorker.Reset();
			return EMessagePopResult::Disconnected;
		}

		const bool bPop = NetworkWorker->TryPopMessage(OutMessageType, OutMessageBodyLength, OutMessageBodyBuffer,
		                                               InMessageBodyBufferSize);

		return bPop ? EMessagePopResult::Pop : EMessagePopResult::Empty;
	}

	FORCEINLINE bool IsConnected() const
	{
		return NetworkWorker.IsValid() ? NetworkWorker->IsConnected() : false;
	}

private:
	FString DisconnectedReason;
	TUniquePtr<FNetworkWorker> NetworkWorker;
};
