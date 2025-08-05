// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "LoginWidget.h"
#include "RatkiniaClientSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"

FText ULoginWidget::GetId() const
{
	return IdInputBox->GetText();
}

FText ULoginWidget::GetPassword() const
{
	return PasswordInputBox->GetText();
}

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::OnLoginButtonPressedHandler);
	RegisterButton->OnClicked.AddDynamic(this, &ULoginWidget::OnRegisterButtonPressedHandler);
	ResetPasswordButton->OnClicked.AddDynamic(this, &ULoginWidget::OnResetPasswordButtonPressedHandler);
	QuitGameButton->OnClicked.AddDynamic(this, &ULoginWidget::OnQuitGameButtonPressedHandler);
}

void ULoginWidget::SetLoginAvailability(const bool bIsAvailable)
{
	IdInputBox->SetIsEnabled(bIsAvailable);
	PasswordInputBox->SetIsEnabled(bIsAvailable);
	LoginButton->SetIsEnabled(bIsAvailable);
	RegisterButton->SetIsEnabled(bIsAvailable);
	ResetPasswordButton->SetIsEnabled(bIsAvailable);
}

void ULoginWidget::OnLoginButtonPressedHandler()
{
	OnLoginButtonPressed.Broadcast(IdInputBox->GetText(), PasswordInputBox->GetText());
}

void ULoginWidget::OnRegisterButtonPressedHandler()
{
	OnRegisterButtonPressed.Broadcast();
}

void ULoginWidget::OnResetPasswordButtonPressedHandler()
{
	OnResetPasswordButtonPressed.Broadcast();
}

void ULoginWidget::OnQuitGameButtonPressedHandler()
{
	OnQuitGameButtonPressed.Broadcast();
}
