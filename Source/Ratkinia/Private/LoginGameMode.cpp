// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#include "LoginGameMode.h"
#include "RatkiniaClientSubsystem.h"
#include "MessageBoxWidget.h"
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "Blueprint/UserWidget.h"

ALoginGameMode::ALoginGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ALoginGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URatkiniaClientSubsystem* const RatkiniaClient{GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()};

	if (RatkiniaClient->GetConnectionState() == ERatkiniaConnectionState::Disconnected)
	{
		if (const FString DisconnectedReason = RatkiniaClient->GetDisconnectedReason();
			!DisconnectedReason.IsEmpty())
		{
			PopupMessageBoxWidget(FText::FromString(DisconnectedReason));
		}
		RatkiniaClient->ClearSession();
		return;
	}

	if (RatkiniaClient->GetConnectionState() != ERatkiniaConnectionState::Connected)
	{
		return;
	}

	if (PostConnectAction)
	{
		PostConnectAction();
		PostConnectAction = nullptr;
	}

	while (const TOptional<TScopedNetworkMessage<FNetworkWorker>> Message = RatkiniaClient->TryPopMessage())
	{
		HandleStc(0, Message->GetMessageType(), Message->GetBodySize(), Message->GetBody());
	}
}

void ALoginGameMode::OnUnknownMessageType(const uint32_t context, const RatkiniaProtocol::StcMessageType messageType)
{
	FString Message{TEXT("서버로부터 알 수 없는 메시지를 수신하였습니다: ")};
	Message.AppendInt(static_cast<int>(messageType));
	Message.Append(TEXT(". 연결을 종료합니다."));
	PopupMessageBoxWidget(FText::FromString(Message));
	
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnParseMessageFailed(const uint32_t context, const RatkiniaProtocol::StcMessageType messageType)
{
	FString Message{TEXT("서버로부터 수신한 메시지 해석에 실패하였습니다. 메시지 번호: ")};
	Message.AppendInt(static_cast<int>(messageType));
	Message.Append(TEXT(". 연결을 종료합니다."));

	PopupMessageBoxWidget(FText::FromString(Message));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnUnhandledMessageType(const uint32_t context, const RatkiniaProtocol::StcMessageType messageType)
{
	FString Message{TEXT("서버로부터 수신한 메시지를 처리하지 못했습니다. 메시지 번호: ")};
	Message.AppendInt(static_cast<int>(messageType));
	Message.Append(TEXT(". 연결을 종료합니다."));

	PopupMessageBoxWidget(FText::FromString(Message));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnLoginResponse(const uint32_t, const bool successful)
{
	if (!successful)
	{
		FString Message{TEXT("로그인에 실패하였습니다.")};
		PopupMessageBoxWidget(FText::FromString(Message));
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}
	PopupMessageBoxWidget(FText::FromString(TEXT("로그인 성공!")));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnRegisterResponse(const uint32_t context,
                                        const bool successful,
                                        const std::string& failed_reason)
{
	if (!successful)
	{
		PopupMessageBoxWidget(FText::FromString(UTF8_TO_TCHAR(failed_reason.c_str())));
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}
	PopupMessageBoxWidget(FText::FromString(TEXT("회원가입 성공!")));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}


void ALoginGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* const PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeUIOnly{});

	OpenLoginWidget();
}

void ALoginGameMode::RatkiniaLogin(const FText Id, const FText Password)
{
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Connect("127.0.0.1", 31415);

	PostConnectAction =
		[this, Id, Password]()
		{
			GetGameInstance()
			->GetSubsystem<URatkiniaClientSubsystem>()
			->LoginRequest(0, TCHAR_TO_UTF8(*Id.ToString()), TCHAR_TO_UTF8(*Password.ToString()));
		};
}

void ALoginGameMode::RatkiniaRegister(const FText Id, const FText Password, const FText PasswordAgain)
{
	if (!Password.EqualTo(PasswordAgain))
	{
		PopupMessageBoxWidget(FText::FromString(TEXT("비밀번호가 일치하지 않습니다.")));
		return;
	}
	
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Connect("127.0.0.1", 31415);

	PostConnectAction =
		[this, Id, Password]()
		{
			GetGameInstance()
			->GetSubsystem<URatkiniaClientSubsystem>()
			->RegisterRequest(0, TCHAR_TO_UTF8(*Id.ToString()), 
			TCHAR_TO_UTF8(*Password.ToString()));
		};
}

void ALoginGameMode::OpenLoginWidget()
{
	ULoginWidget* const LoginWidget = CreateWidget<ULoginWidget>(GetWorld(), LoginWidgetClass);
	check(IsValid(LoginWidget));
	LoginWidget->OnLoginButtonPressed.AddDynamic(this, &ALoginGameMode::RatkiniaLogin);
	LoginWidget->OnRegisterButtonPressed.AddDynamic(this, &ALoginGameMode::OpenRegisterWidget);
	LoginWidget->OnResetPasswordButtonPressed.AddDynamic(this, &ALoginGameMode::OpenResetPasswordWidget);
	LoginWidget->OnQuitGameButtonPressed.AddDynamic(this, &ALoginGameMode::OpenReallyQuitGameWidget);
	LoginWidget->AddToViewport();
}

void ALoginGameMode::OpenRegisterWidget()
{
	URegisterWidget* const RegisterWidget = CreateWidget<URegisterWidget>(GetWorld(), RegisterWidgetClass);
	check(IsValid(RegisterWidget));

	RegisterWidget->OnRegisterButtonClicked.AddDynamic(this, &ALoginGameMode::RatkiniaRegister);
	RegisterWidget->OnCancelButtonClicked.AddDynamic(this, &ALoginGameMode::OpenLoginWidget);
	RegisterWidget->AddToViewport();
}

void ALoginGameMode::OpenResetPasswordWidget()
{
}

void ALoginGameMode::OpenReallyQuitGameWidget()
{
}

void ALoginGameMode::PopupMessageBoxWidget(const FText Text)
{
	UMessageBoxWidget* const MessageBoxWidget{CreateWidget<UMessageBoxWidget>(GetWorld(), MessageBoxWidgetClass)};
	check(IsValid(MessageBoxWidget));

	MessageBoxWidget->SetMessage(Text);
	MessageBoxWidget->AddToViewport();
}
