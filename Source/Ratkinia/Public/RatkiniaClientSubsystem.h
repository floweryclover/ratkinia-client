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
                                                    public RatkiniaProtocol::CtsProxy<URatkiniaClientSubsystem>
{
	GENERATED_BODY()

public:
	explicit URatkiniaClientSubsystem();

	explicit URatkiniaClientSubsystem(FVTableHelper& Helper);

	virtual ~URatkiniaClientSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	bool Connect(const FString& ServerAddress, int32_t ServerPort);

	void Disconnect();

	template <typename TMessage>
	void WriteMessage(const uint64_t, const RatkiniaProtocol::CtsMessageType MessageType, const TMessage& Message)
	{
		if (!NetworkWorker.IsValid())
		{
			return;
		}

		NetworkWorker->Send(Message, MessageType);
	}

	FORCEINLINE EMessagePopResult Receive(uint16& OutMessageType,
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
			DisconnectedReason = NetworkWorker->GetEndReason();
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

	FORCEINLINE bool IsStopped() const
	{
		if (!NetworkWorker.IsValid() || !NetworkWorker->IsConnected())
		{
			return false;
		}

		return NetworkWorker->IsStopped();
	}

	FORCEINLINE FString GetStoppedReason() const
	{
		if (!NetworkWorker.IsValid())
		{
			return {};
		}
		return NetworkWorker->GetEndReason();
	}

private:
	FString DisconnectedReason;
	TUniquePtr<FNetworkWorker> NetworkWorker;
};
