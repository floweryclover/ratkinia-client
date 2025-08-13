// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "CharacterSelectGameMode.h"
#include "RatkiniaClientSubsystem.h"
#include "Kismet/GameplayStatics.h"

void ACharacterSelectGameMode::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	URatkiniaClientSubsystem* const Client = GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>();
	if (Client->GetConnectionState() != ERatkiniaConnectionState::Connected)
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/CharacterSelect/CharacterSelect"), true);
		return;
	}
}

void ACharacterSelectGameMode::OnUnknownMessageType(const RatkiniaProtocol::StcMessageType MessageType)
{
}

void ACharacterSelectGameMode::OnParseMessageFailed(const RatkiniaProtocol::StcMessageType MessageType)
{
}

void ACharacterSelectGameMode::OnUnhandledMessageType(const RatkiniaProtocol::StcMessageType MessageType)
{
}
