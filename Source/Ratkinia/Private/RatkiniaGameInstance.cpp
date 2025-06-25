// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaGameInstance.h"

void URatkiniaGameInstance::Init()
{
	Super::Init();
}

void URatkiniaGameInstance::SetLastDisconnectedReason(FText Reason)
{
	LastDisconnectedReason = MoveTemp(Reason);
}

FText URatkiniaGameInstance::PopLastDisconnectedReason()
{
	FText Temp { MoveTemp(LastDisconnectedReason)};
	LastDisconnectedReason = FText::GetEmpty();
	return Temp;
}