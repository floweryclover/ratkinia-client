// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginUserWidget.generated.h"

class UButton;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class ULoginUserWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void OnLoginSuccess();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoginFailure(const FString& Reason);

protected:
	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* Id;

	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* Password;

	UPROPERTY(meta=(BindWidget))
	UButton* LoginButton;

private:
	UFUNCTION()
	void OnLoginButtonPressed();
};
