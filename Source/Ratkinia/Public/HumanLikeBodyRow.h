// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "HumanLikeBodyRow.generated.h"

USTRUCT(BlueprintType)
struct RATKINIA_API FHumanLikeBodyRow final : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
};
