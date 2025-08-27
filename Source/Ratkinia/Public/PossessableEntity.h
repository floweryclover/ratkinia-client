// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entity.h"
#include "GameFramework/Character.h"
#include "PossessableEntity.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class APossessableEntity final : public ACharacter, public IEntity
{
	GENERATED_BODY()

public:
	explicit APossessableEntity();
	
	virtual void SetEntityId(const uint32 Id) override
	{
		EntityId = Id;
	}

	virtual uint32 GetEntityId() const override
	{
		return EntityId;
	}

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

private:
	uint32 EntityId;
};
