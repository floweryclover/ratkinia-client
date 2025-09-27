// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PossessableEntity.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class APossessableEntity final : public ACharacter
{
	GENERATED_BODY()

public:
	explicit APossessableEntity();
	
	void SetEntityId(const uint32 Id)
	{
		EntityId = Id;
	}

	uint32 GetEntityId() const
	{
		return EntityId;
	}

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

private:
	uint32 EntityId;
};
