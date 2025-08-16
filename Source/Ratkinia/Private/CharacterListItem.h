// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterListItem.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class UCharacterListItem final : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CharacterName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SelectButton;
};
