// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateCharacterWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateCharacterWidget_CreateButtonClicked, FText, Name);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCreateCharacterWidget_CancelButtonClicked);


class UButton;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class UCreateCharacterWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FCreateCharacterWidget_CreateButtonClicked OnCreateButtonClicked;

	UPROPERTY()
	FCreateCharacterWidget_CancelButtonClicked OnCancelButtonClicked;
	
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Name;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CreateButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;

	UFUNCTION()
	void OnCreateButtonClickedHandler();

	UFUNCTION()
	void OnCancelButtonClickedHandler();
};
