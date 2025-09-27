// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EntityComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RATKINIACOMPONENT_API UEntityComponent final : public USceneComponent
{
	GENERATED_BODY()

public:
	constexpr static uint32 NullEntityId = UINT32_MAX;

	void Init(const uint32 Id)
	{
		check(EntityId == NullEntityId);
		EntityId = Id;
	}

	uint32 GetEntityId() const
	{
		return EntityId;
	}

private:
	uint32 EntityId = NullEntityId;
};
