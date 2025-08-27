// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "PossessableEntity.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

APossessableEntity::APossessableEntity()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}
