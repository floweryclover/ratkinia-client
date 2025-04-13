// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class ULoginUserWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Login(const FString& ServerAddress, int32 ServerPort);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoginSuccess();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoginFailure(const FString& Reason);
};
