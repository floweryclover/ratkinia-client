// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaGameInstance.h"
#include "RatkiniaClientSubsystem.h"

void URatkiniaGameInstance::Init()
{
	Super::Init();

	URatkiniaClientSubsystem* ClientSubsystem = GetSubsystem<URatkiniaClientSubsystem>();
}