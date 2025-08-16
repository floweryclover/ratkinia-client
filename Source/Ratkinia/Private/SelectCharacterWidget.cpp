// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "SelectCharacterWidget.h"

#include "Components/Button.h"

void USelectCharacterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateCharacterButton->OnClicked.AddDynamic(this, &USelectCharacterWidget::OnCreateCharacterButtonClickedHandler);
}

void USelectCharacterWidget::OnCreateCharacterButtonClickedHandler()
{
	OnCreateCharacterButtonClicked.Broadcast();
}
