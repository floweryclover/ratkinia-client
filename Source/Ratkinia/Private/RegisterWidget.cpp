// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RegisterWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void URegisterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RegisterButton->OnClicked.AddDynamic(this, &URegisterWidget::OnRegisterButtonClickedHandler);
	CancelButton->OnClicked.AddDynamic(this, &URegisterWidget::OnCancelButtonClickedHandler);
}

void URegisterWidget::OnRegisterButtonClickedHandler()
{
	OnRegisterButtonClicked.Broadcast(IdInputBox->GetText(), PasswordInputBox->GetText(), PasswordAgainInputBox->GetText());
}

void URegisterWidget::OnCancelButtonClickedHandler()
{
	OnCancelButtonClicked.Broadcast();
}
