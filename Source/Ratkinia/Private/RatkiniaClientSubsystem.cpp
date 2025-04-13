// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaClientSubsystem.h"
#include <WS2tcpip.h>

URatkiniaClientSubsystem::URatkiniaClientSubsystem()
	: ClientSocket{INVALID_SOCKET}
{
}

void URatkiniaClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WSADATA WsaData{};
	check(WSAStartup(MAKEWORD(2, 2), &WsaData) == 0);
}

void URatkiniaClientSubsystem::Deinitialize()
{
	Super::Deinitialize();
	WSACleanup();
}


void URatkiniaClientSubsystem::Login(const FString& ServerAddress, int32_t ServerPort,
                                     TFunction<void(const FString&)> OnFailure,
                                     TFunction<void()> OnSuccess)
{
	if (ClientSocket != INVALID_SOCKET)
	{
		OnFailure(TEXT("이미 서버에 접속되어 있거나 접속 중입니다."));
		return;
	}

	SOCKADDR_IN addrIn{};
	addrIn.sin_family = AF_INET;
	addrIn.sin_port = htons(static_cast<unsigned short>(ServerPort));
	if (int Result = inet_pton(AF_INET, TCHAR_TO_UTF8(*ServerAddress), &addrIn.sin_addr); Result != 1)
	{
		FString Reason;
		if (Result == 0)
		{
			Reason = TEXT("서버 주소 문자열이 유효한 IPv4 형식이 아닙니다.");
		}
		else
		{
			Reason = TEXT("서버 주소 문자열 해석에 실패하였습니다: ");
			Reason += FString::FromInt(WSAGetLastError());
		}
		OnFailure(Reason);
		return;
	}

	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET)
	{
		FString Reason{TEXT("클라이언트 소켓 생성에 실패하였습니다: ")};
		Reason += FString::FromInt(WSAGetLastError());
		OnFailure(Reason);
		return;
	}

	AsyncTask(ENamedThreads::Type::AnyThread, [this, addrIn, OnFailure, OnSuccess]
	{
		const int Result = connect(ClientSocket, reinterpret_cast<const sockaddr*>(&addrIn), sizeof(SOCKADDR_IN));
		const int ErrorCode = Result == SOCKET_ERROR ? WSAGetLastError() : 0;
		AsyncTask(ENamedThreads::Type::GameThread, [this, Result, ErrorCode, OnFailure, OnSuccess]
		{
			if (Result == SOCKET_ERROR)
			{
				FString Reason{"서버 접속에 실패하였습니다: "};
				Reason += FString::FromInt(ErrorCode);
				closesocket(ClientSocket);
				ClientSocket = INVALID_SOCKET;
				OnFailure(Reason);
			}
			else
			{
				OnSuccess();
			}
		});
	});
}
