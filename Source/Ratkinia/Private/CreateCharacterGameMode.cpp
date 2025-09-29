// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "CreateCharacterGameMode.h"

#include "CreateCharacterWidget.h"
#include "MessageBoxWidget.h"
#include "RatkiniaClientSubsystem.h"
#include "RatkiniaGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Ratkinia/Ratkinia.h"

using namespace RatkiniaProtocol;

ACreateCharacterGameMode::ACreateCharacterGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ACreateCharacterGameMode::BeginPlay()
{
	Super::BeginPlay();

	UCreateCharacterWidget* const CreateCharacterWidget = CreateWidget<UCreateCharacterWidget>(
		GetWorld(), CreateCharacterWidgetClass);
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

void ACreateCharacterGameMode::OnUnknownMessageType(StcMessageType MessageType)
{
	checkNoEntry();
}

void ACreateCharacterGameMode::OnParseMessageFailed(StcMessageType MessageType)
{
	checkNoEntry();
}

void ACreateCharacterGameMode::OnUnhandledMessageType(StcMessageType MessageType)
{
	checkNoEntry();
}

void ACreateCharacterGameMode::OnCreateCharacterResponse(const CreateCharacterResponse_CreateCharacterResult Successful)
{
	if (Successful == CreateCharacterResponse_CreateCharacterResult_DuplicateName)
	{
		PopupMessageBoxWidget(FText::FromString(TEXT("이미 사용중인 이름입니다.")));
	}
	else if (Successful == CreateCharacterResponse_CreateCharacterResult_InvalidNameLength)
	{
		PopupMessageBoxWidget(FText::FromString(TEXT("이름의 길이 또는 형식이 잘못되었습니다.")));
	}
	else if (Successful == CreateCharacterResponse_CreateCharacterResult_UnknownError)
	{
		PopupMessageBoxWidget(FText::FromString(TEXT("알 수 없는 에러가 발생하였습니다.")));
	}
	else
	{
		PopupMessageBoxWidget(FText::FromString(TEXT("캐릭터 생성에 성공하였습니다.")));
	}
}

void ACreateCharacterGameMode::OnNotify(const Notify_Type Type, const FString Text)
{
	if (Type == Notify_Type_Fatal)
	{
		Cast<URatkiniaGameInstance>(GetGameInstance())->FatalNotifications.Add(Text);
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}

	PopupMessageBoxWidget(FText::FromString(Text));
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

void ACreateCharacterGameMode::PopupMessageBoxWidget(const FText Text)
{
	UMessageBoxWidget* const MessageBoxWidget{CreateWidget<UMessageBoxWidget>(GetWorld(), MessageBoxWidgetClass)};
	check(IsValid(MessageBoxWidget));

	MessageBoxWidget->SetMessage(Text);
	MessageBoxWidget->AddToViewport();
}
