// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "CreateCharacterGameMode.h"

#include "CreateCharacterWidget.h"
#include "RatkiniaClientSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Ratkinia/Ratkinia.h"

ACreateCharacterGameMode::ACreateCharacterGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ACreateCharacterGameMode::BeginPlay()
{
	Super::BeginPlay();

	UCreateCharacterWidget* const CreateCharacterWidget = CreateWidget<UCreateCharacterWidget>(GetWorld(), CreateCharacterWidgetClass);
	check(IsValid(CreateCharacterWidget));
	CreateCharacterWidget->OnCreateButtonClicked.AddDynamic(this, &ACreateCharacterGameMode::CreateCharacter);
	CreateCharacterWidget->OnCancelButtonClicked.AddDynamic(this, &ACreateCharacterGameMode::OpenSelectCharacterLevel);
	CreateCharacterWidget->AddToViewport();
}

void ACreateCharacterGameMode::CreateCharacter(const FText Name)
{
	UE_LOG(LogRatkinia, Log, TEXT("Creating character"));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->CreateCharacter(Name.ToString());
}

void ACreateCharacterGameMode::OnUnknownMessageType(RatkiniaProtocol::StcMessageType MessageType)
{
	checkNoEntry();
}

void ACreateCharacterGameMode::OnParseMessageFailed(RatkiniaProtocol::StcMessageType MessageType)
{
	checkNoEntry();
}

void ACreateCharacterGameMode::OnUnhandledMessageType(RatkiniaProtocol::StcMessageType MessageType)
{
	checkNoEntry();
}

void ACreateCharacterGameMode::OnCreateCharacterResponse(
	RatkiniaProtocol::CreateCharacterResponse_CreateCharacterResult Successful)
{
	UE_LOG(LogRatkinia, Log, TEXT("결과: %d"), Successful);
}

void ACreateCharacterGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URatkiniaClientSubsystem* const Client = GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>();
	if (Client->GetConnectionState() != ERatkiniaConnectionState::Connected)
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/Login/Login"), true);
	}

	while (const auto MessagePeekResult = Client->TryPeekMessage())
	{
		HandleStc(MessagePeekResult->MessageType, MessagePeekResult->BodySize, MessagePeekResult->Body);
		Client->PopMessage(*MessagePeekResult);
	}
}

void ACreateCharacterGameMode::OpenSelectCharacterLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/SelectCharacter/SelectCharacter"));
}
