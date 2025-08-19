// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "SelectCharacterWidget.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"

void USelectCharacterWidget::ClearCharacters()
{
	CharacterList->ClearChildren();
}

void USelectCharacterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateCharacterButton->OnClicked.AddDynamic(this, &USelectCharacterWidget::OnCreateCharacterButtonClickedHandler);
}

void USelectCharacterWidget::OnCreateCharacterButtonClickedHandler()
{
	OnCreateCharacterButtonClicked.Broadcast();
}
