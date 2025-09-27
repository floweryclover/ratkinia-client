// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "IEcsComponent.h"

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HumanLikeBodyComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RATKINIACOMPONENT_API UHumanLikeBodyComponent final : public USkeletalMeshComponent, public IEcsComponent
{
	GENERATED_BODY()
	
public:
	ECSCOMPONENT(HumanLikeBody)

	virtual void UpdateEcsComponent(const RatkiniaProtocol::ComponentVariant& ComponentVariant) override;
};
