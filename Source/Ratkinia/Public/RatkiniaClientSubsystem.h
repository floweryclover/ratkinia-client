// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include <WinSock2.h>
#include "CoreMinimal.h"
#include "RatkiniaClientSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class RATKINIA_API URatkiniaClientSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URatkiniaClientSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	void Login(const FString& ServerAddress, int32_t ServerPort, TFunction<void(const FString&)> OnFailure,
	           TFunction<void()> OnSuccess);

	void SetHandlers(TFunction<void(const FString&)> FatalHandler, TFunction<void(const FString&)> ErrorHandler)
	{
		OnFatal = MoveTemp(FatalHandler);
		OnError = MoveTemp(ErrorHandler);
	}

private:
	TFunction<void(const FString&)> OnFatal;
	TFunction<void(const FString&)> OnError;

	SOCKET ClientSocket;
};
