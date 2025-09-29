// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "SelectCharacterGameMode.h"

#include "MessageBoxWidget.h"
#include "SelectCharacterWidget.h"
#include "RatkiniaClientSubsystem.h"
#include "RatkiniaGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Ratkinia/Ratkinia.h"

using namespace RatkiniaProtocol;

ASelectCharacterGameMode::ASelectCharacterGameMode()
	: bBreakMessagePopLoop{false}
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASelectCharacterGameMode::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	URatkiniaClientSubsystem* const Client = GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>();
	if (Client->GetConnectionState() != ERatkiniaConnectionState::Connected)
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/Login/Login"), true);
	}

	bBreakMessagePopLoop = false;
	while (const auto MessagePeekResult = Client->TryPeekMessage())
	{
		if (bBreakMessagePopLoop)
		{
			break;
		}
		HandleStc(MessagePeekResult->MessageType, MessagePeekResult->BodySize, MessagePeekResult->Body);
		Client->PopMessage(*MessagePeekResult);
	}
}

void ASelectCharacterGameMode::OnUnknownMessageType(const StcMessageType MessageType)
{
}

void ASelectCharacterGameMode::OnParseMessageFailed(const StcMessageType MessageType)
{
}

void ASelectCharacterGameMode::OnUnhandledMessageType(const StcMessageType MessageType)
{
}

void ASelectCharacterGameMode::OnSendMyCharacters(const TArrayView<const SendMyCharacters_Data* const> CharacterLoadDatas)
{
	SelectCharacterWidget->ClearCharacters();
	for (const SendMyCharacters_Data* const Data : CharacterLoadDatas)
	{
		SelectCharacterWidget->AddCharacter(Data->id(), UTF8_TO_TCHAR(Data->name().c_str()));
	}
}

void ASelectCharacterGameMode::OnOpenWorld()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/World"));
	bBreakMessagePopLoop = true;
}

void ASelectCharacterGameMode::OnNotify(const Notify_Type Type, const FString Text)
{
	if (Type == Notify_Type_Fatal)
	{
		Cast<URatkiniaGameInstance>(GetGameInstance())->FatalNotifications.Add(Text);
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}

	PopupMessageBoxWidget(FText::FromString(Text));
}

void ASelectCharacterGameMode::BeginPlay()
{
	Super::BeginPlay();

	SelectCharacterWidget = CreateWidget<USelectCharacterWidget>(GetWorld(), SelectCharacterWidgetClass);
	check(IsValid(SelectCharacterWidget));
	SelectCharacterWidget->OnCreateCharacterButtonClicked.AddDynamic(this, &ASelectCharacterGameMode::OpenCreateCharacterLevel);
	SelectCharacterWidget->OnCharacterSelected.AddDynamic(this, &ASelectCharacterGameMode::SelectCharacter);
	SelectCharacterWidget->AddToViewport();

	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->LoadMyCharacters();
}

void ASelectCharacterGameMode::OpenCreateCharacterLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/SelectCharacter/CreateCharacter/CreateCharacter"));
}

void ASelectCharacterGameMode::SelectCharacter(const int32 Id)
{
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->SelectCharacter(Id);
}

void ASelectCharacterGameMode::PopupMessageBoxWidget(const FText Text)
{
	UMessageBoxWidget* const MessageBoxWidget{CreateWidget<UMessageBoxWidget>(GetWorld(), MessageBoxWidgetClass)};
	check(IsValid(MessageBoxWidget));

	MessageBoxWidget->SetMessage(Text);
	MessageBoxWidget->AddToViewport();
}
