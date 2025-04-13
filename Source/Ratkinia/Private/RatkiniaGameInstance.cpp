// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaGameInstance.h"
#include "RatkiniaClientSubsystem.h"
#include "Ratkinia/Ratkinia.h"

void URatkiniaGameInstance::Init()
{
	Super::Init();

	URatkiniaClientSubsystem* ClientSubsystem = GetSubsystem<URatkiniaClientSubsystem>();
	ClientSubsystem->SetHandlers([this](const FString& Reason) { OnFatal(Reason); },
	                             [this](const FString& Reason) { OnError(Reason); });
}

void URatkiniaGameInstance::OnFatal(const FString& Reason)
{
	UE_LOG(LogRatkinia, Fatal, TEXT("%s"), *Reason);
}

void URatkiniaGameInstance::OnError(const FString& Reason)
{
	UE_LOG(LogRatkinia, Error, TEXT("%s"), *Reason);
}
