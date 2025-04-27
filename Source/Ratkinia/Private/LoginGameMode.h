// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LoginGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ALoginGameMode final : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	explicit ALoginGameMode();
	
	virtual ~ALoginGameMode() override;
	
	virtual void Tick(float DeltaSeconds) override;

private:
	TUniquePtr<char[]> MessageBodyBuffer;
	bool bIsConnected;
};
