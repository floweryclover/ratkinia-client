// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RatkiniaGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class RATKINIA_API URatkiniaGameInstance final : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

private:
	void OnFatal(const FString& Reason);
	void OnError(const FString& Reason);
};
