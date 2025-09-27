// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTableSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class RATKINIADATA_API UDataTableSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	const TObjectPtr<UDataTable> HumanBodyBody;
	
	explicit UDataTableSubsystem();
};
