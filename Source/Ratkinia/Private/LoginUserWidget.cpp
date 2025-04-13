// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "LoginUserWidget.h"
#include "RatkiniaClientSubsystem.h"

void ULoginUserWidget::Login(const FString& ServerAddress, int32 ServerPort)
{
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Login(ServerAddress, ServerPort,
	                                                                   [this](const FString& Reason)
	                                                                   {
		                                                                   OnLoginFailure(Reason);
	                                                                   },
	                                                                   [this] { OnLoginSuccess(); });
}
