// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#include "LoginGameMode.h"
#include "RatkiniaClientSubsystem.h"
#include "MessageBoxWidget.h"
#include "LoginWidget.h"
#include "RatkiniaGameInstance.h"
#include "RegisterWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

using namespace RatkiniaProtocol;

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

	while (const TOptional<FMessagePeekResult> Message = RatkiniaClient->TryPeekMessage())
	{
		HandleStc(Message->MessageType, Message->BodySize, Message->Body);
		RatkiniaClient->PopMessage(*Message);
	}
}

void ALoginGameMode::OnUnknownMessageType(const StcMessageType MessageType)
{
	FString Message{TEXT("서버로부터 알 수 없는 메시지를 수신하였습니다: ")};
	Message.AppendInt(static_cast<int>(MessageType));
	Message.Append(TEXT(". 연결을 종료합니다."));
	PopupMessageBoxWidget(FText::FromString(Message));
	
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnParseMessageFailed(const StcMessageType MessageType)
{
	FString Message{TEXT("서버로부터 수신한 메시지 해석에 실패하였습니다. 메시지 번호: ")};
	Message.AppendInt(static_cast<int>(MessageType));
	Message.Append(TEXT(". 연결을 종료합니다."));

	PopupMessageBoxWidget(FText::FromString(Message));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnUnhandledMessageType(const StcMessageType MessageType)
{
	FString Message{TEXT("서버로부터 수신한 메시지를 처리하지 못했습니다. 메시지 번호: ")};
	Message.AppendInt(static_cast<int>(MessageType));
	Message.Append(TEXT(". 연결을 종료합니다."));

	PopupMessageBoxWidget(FText::FromString(Message));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnLoginResponse(const LoginResponse_LoginResult Result)
{
	if (Result != LoginResponse_LoginResult_Success)
	{
		FString Message{TEXT("로그인에 실패하였습니다: ")};
		if (Result == LoginResponse_LoginResult_DuplicateAccount)
		{
			Message += TEXT("이미 접속 중인 계정입니다.");
		}
		else if (Result == LoginResponse_LoginResult_DuplicateContext)
		{
			Message += TEXT("해당 PC에서 중복 접속이 감지되었습니다.");
		}
		else
		{
			Message += TEXT("로그인에 실패하였습니다.");	
		}
		
		PopupMessageBoxWidget(FText::FromString(Message));
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}
	PopupMessageBoxWidget(FText::FromString(TEXT("로그인 성공!")));
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/SelectCharacter/SelectCharacter"));
}

void ALoginGameMode::OnRegisterResponse(const bool Successful,
                                        const FString FailedReason)
{
	if (!Successful)
	{
		PopupMessageBoxWidget(FText::FromString(FailedReason));
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}
	PopupMessageBoxWidget(FText::FromString(TEXT("회원가입 성공!")));
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
}

void ALoginGameMode::OnNotify(const Notify_Type Type, const FString Text)
{
	if (Type == Notify_Type_Fatal)
	{
		Cast<URatkiniaGameInstance>(GetGameInstance())->FatalNotifications.Add(Text);
		GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession();
		return;
	}

	PopupMessageBoxWidget(FText::FromString(Text));
}


void ALoginGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* const PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(FInputModeUIOnly{});

	OpenLoginWidget();

	TArray<FString>& FatalNotifications = Cast<URatkiniaGameInstance>(GetGameInstance())->FatalNotifications;
	for (const FString FatalNotification : FatalNotifications)
	{
		PopupMessageBoxWidget(FText::FromString(FatalNotification));
	}
	FatalNotifications.Empty();
}

void ALoginGameMode::RatkiniaLogin(const FText Id, const FText Password)
{
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Connect("127.0.0.1", 31415);

	PostConnectAction =
		[this, Id, Password]
		{
			GetGameInstance()
			->GetSubsystem<URatkiniaClientSubsystem>()
			->LoginRequest(TCHAR_TO_UTF8(*Id.ToString()), TCHAR_TO_UTF8(*Password.ToString()));
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
			->RegisterRequest(TCHAR_TO_UTF8(*Id.ToString()), 
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
