// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity.h"
#include "GameFramework/Actor.h"
#include "NormalEntity.generated.h"

UCLASS()
class RATKINIA_API ANormalEntity final : public AActor, public IEntity
{
	GENERATED_BODY()

public:
	virtual void SetEntityId(const uint32 Id) override
	{
		EntityId = Id;
	}

	virtual uint32 GetEntityId() const override
	{
		return EntityId;
	}
	
private:
	uint32 EntityId;
};
