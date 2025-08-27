// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Entity.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UEntity : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RATKINIA_API IEntity
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetEntityId(uint32 Id) = 0;

	virtual uint32 GetEntityId() const = 0;
};