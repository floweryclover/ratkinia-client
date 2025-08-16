// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "CreateCharacterWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void UCreateCharacterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateButton->OnClicked.AddDynamic(this, &UCreateCharacterWidget::OnCreateButtonClickedHandler);
	CancelButton->OnClicked.AddDynamic(this, &UCreateCharacterWidget::OnCancelButtonClickedHandler);
}

void UCreateCharacterWidget::OnCreateButtonClickedHandler()
{
	OnCreateButtonClicked.Broadcast(Name->GetText());
}

void UCreateCharacterWidget::OnCancelButtonClickedHandler()
{
	OnCancelButtonClicked.Broadcast();
}
