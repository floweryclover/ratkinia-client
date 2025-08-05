// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RegisterWidget.generated.h"

class UButton;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRegisterButtonClicked, FText, Id, FText, Password, FText, PasswordAgain);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelButtonClicked);

/**
 * 
 */
UCLASS()
class URegisterWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnRegisterButtonClicked OnRegisterButtonClicked;
	FOnCancelButtonClicked OnCancelButtonClicked;
	
private:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> IdInputBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> PasswordInputBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> PasswordAgainInputBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RegisterButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;

	UFUNCTION()
	void OnRegisterButtonClickedHandler();

	UFUNCTION()
	void OnCancelButtonClickedHandler();
};
