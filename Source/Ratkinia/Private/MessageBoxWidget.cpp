// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "MessageBoxWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMessageBoxWidget::SetMessage(FText InMessage)
{
	Message->SetText(std::move(InMessage));
}

void UMessageBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OkButton->OnClicked.AddDynamic(this, &UMessageBoxWidget::OnOkButtonPressedHandler);
}

void UMessageBoxWidget::OnOkButtonPressedHandler()
{
	OnOkButtonPressed.Broadcast();
	RemoveFromParent();
}
