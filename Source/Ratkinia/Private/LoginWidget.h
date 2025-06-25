// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLoginButtonPressed, FText, Id, FText, Password);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRegisterButtonPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetPasswordButtonPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuitGameButtonPressed);

class UButton;
class UEditableTextBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ULoginWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FLoginButtonPressed OnLoginButtonPressed;

	UPROPERTY()
	FRegisterButtonPressed OnRegisterButtonPressed;

	UPROPERTY()
	FResetPasswordButtonPressed OnResetPasswordButtonPressed;
	
	UPROPERTY()
	FQuitGameButtonPressed OnQuitGameButtonPressed;

	virtual void NativeConstruct() override;
	
	void SetLoginAvailability(bool bIsAvailable);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> IdInputBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> PasswordInputBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RegisterButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ResetPasswordButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> QuitGameButton;

private:
	UFUNCTION()
	void OnLoginButtonPressedHandler();

	UFUNCTION()
	void OnRegisterButtonPressedHandler();

	UFUNCTION()
	void OnResetPasswordButtonPressedHandler();

	UFUNCTION()
	void OnQuitGameButtonPressedHandler();
};
