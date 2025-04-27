// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "LoginUserWidget.h"
#include "RatkiniaClientSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void ULoginUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoginButton->OnClicked.AddDynamic(this, &ULoginUserWidget::OnLoginButtonPressed);
}

void ULoginUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void ULoginUserWidget::OnLoginButtonPressed()
{
	if (GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Connect(
		"127.0.0.1", 31415))
	{
		StatusText->SetText(FText::FromString(TEXT("연결 중...")));
	}
}
