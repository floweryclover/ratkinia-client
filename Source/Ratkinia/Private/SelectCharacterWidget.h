// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectCharacterWidget.generated.h"

class UButton;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectedCharacterWidget_OnCreateCharacterButtonClicked);

class UVerticalBox;
/**
 * 
 */
UCLASS()
class USelectCharacterWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FSelectedCharacterWidget_OnCreateCharacterButtonClicked OnCreateCharacterButtonClicked;

protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> CharacterList;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CreateCharacterButton;

	UFUNCTION()
	void OnCreateCharacterButtonClickedHandler();
};
