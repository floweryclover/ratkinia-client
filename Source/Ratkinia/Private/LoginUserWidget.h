// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginUserWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ULoginUserWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* Id;

	UPROPERTY(meta=(BindWidget))
	UEditableTextBox* Password;

	UPROPERTY(meta=(BindWidget))
	UButton* LoginButton;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* StatusText;

private:
	UFUNCTION()
	void OnLoginButtonPressed();
};
