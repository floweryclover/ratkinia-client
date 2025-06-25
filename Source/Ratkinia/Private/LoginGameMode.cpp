// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#include "LoginGameMode.h"
#include "RatkiniaClientSubsystem.h"
#include "MessageBoxWidget.h"
#include "LoginWidget.h"
#include "Blueprint/UserWidget.h"

ALoginGameMode::ALoginGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ALoginGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URatkiniaClientSubsystem* const RatkiniaClient{ GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>() };

	if (const FString DisconnectedReason{RatkiniaClient->GetDisconnectedReason()}; !DisconnectedReason.IsEmpty())
	{
		RatkiniaClient->ClearSession();
		PopupMessageBoxWidget(FText::FromString(DisconnectedReason));
		return;
	}

	LoginWidget->SetLoginAvailability(RatkiniaClient->GetConnectionState() == ERatkiniaConnectionState::NotConnected || RatkiniaClient->GetConnectionState() == ERatkiniaConnectionState::Disconnected);

	if (RatkiniaClient->GetConnectionState() != ERatkiniaConnectionState::Connected)
	{
		return;
	}

	if (!bLoginRequested)
	{
		bLoginRequested = true;

		RatkiniaClient->LoginRequest(0,
		                             TCHAR_TO_UTF8(*RequestedLoginId.ToString()),
		                             TCHAR_TO_UTF8(*RequestedLoginPassword.ToString()));
	}

	while (const TOptional<TScopedNetworkMessage<FNetworkWorker>> Message = RatkiniaClient->TryPopMessage())
	{
		HandleStc(0, Message->GetMessageType(), Message->GetBodySize(), Message->GetBody());
	}
}

void ALoginGameMode::OnUnknownMessageType(const uint64_t context, const RatkiniaProtocol::StcMessageType messageType)
{
	FString Message{TEXT("서버로부터 알 수 없는 메시지를 수신하였습니다: ")};
	Message.AppendInt(static_cast<int>(messageType));
	Message.Append(". 연결을 종료합니다.");

	PopupMessageBoxWidget(FText::FromString(Message));
}

void ALoginGameMode::OnParseMessageFailed(const uint64_t context, const RatkiniaProtocol::StcMessageType messageType)
{
	FString Message{TEXT("서버로부터 수신한 메시지 해석에 실패하였습니다. 메시지 번호: ")};
	Message.AppendInt(static_cast<int>(messageType));
	Message.Append(". 연결을 종료합니다.");

	PopupMessageBoxWidget(FText::FromString(Message));
}

void ALoginGameMode::OnUnhandledMessageType(const uint64_t context, const RatkiniaProtocol::StcMessageType messageType)
{
	FString Message{TEXT("서버로부터 수신한 메시지를 처리하지 못했습니다. 메시지 번호: ")};
	Message.AppendInt(static_cast<int>(messageType));
	Message.Append(". 연결을 종료합니다.");

	PopupMessageBoxWidget(FText::FromString(Message));
}

void ALoginGameMode::OnLoginResponse(const uint64_t context, const bool successful, const std::string& failure_reason)
{
	if (!successful)
	{
		FString Message{TEXT("로그인에 실패하였습니다. ")};
		Message.Append(UTF8_TO_TCHAR(failure_reason.c_str()));

		PopupMessageBoxWidget(FText::FromString(Message));
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}
	PopupMessageBoxWidget(FText::FromString(TEXT("로그인 성공!")));
}

void ALoginGameMode::OnRegisterResponse(uint64_t context,
                                        const RatkiniaProtocol::RegisterResponse_FailedReason failed_reason)
{
}

void ALoginGameMode::ConnectAndLogin(FText Id, FText Password)
{
	RequestedLoginId = MoveTemp(Id);
	RequestedLoginPassword = MoveTemp(Password);

	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Connect("127.0.0.1", 31415);

	bLoginRequested = false;
}

void ALoginGameMode::OpenRegisterWidget()
{
}

void ALoginGameMode::OpenResetPasswordWidget()
{
}

void ALoginGameMode::OpenReallyQuitGameWidget()
{
}

void ALoginGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* const PlayerController{ GetWorld()->GetFirstPlayerController()};
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeUIOnly{});

	LoginWidget = CreateWidget<ULoginWidget>(GetWorld(), LoginWidgetClass);
	check(IsValid(LoginWidget));
	LoginWidget->OnLoginButtonPressed.AddDynamic(this, &ALoginGameMode::ConnectAndLogin);
	LoginWidget->OnRegisterButtonPressed.AddDynamic(this, &ALoginGameMode::OpenRegisterWidget);
	LoginWidget->OnResetPasswordButtonPressed.AddDynamic(this, &ALoginGameMode::OpenResetPasswordWidget);
	LoginWidget->OnQuitGameButtonPressed.AddDynamic(this, &ALoginGameMode::OpenReallyQuitGameWidget);
	LoginWidget->AddToViewport();
}

void ALoginGameMode::PopupMessageBoxWidget(FText Text)
{
	UMessageBoxWidget* const MessageBoxWidget{CreateWidget<UMessageBoxWidget>(GetWorld(), MessageBoxWidgetClass)};
	check(IsValid(MessageBoxWidget));

	MessageBoxWidget->SetMessage(MoveTemp(Text));
	MessageBoxWidget->AddToViewport();
}
