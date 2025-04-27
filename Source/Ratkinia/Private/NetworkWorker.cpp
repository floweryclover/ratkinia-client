#include "NetworkWorker.h"
#include <WS2tcpip.h>

FNetworkWorker::FNetworkWorker(uint64 BufferCapacity, const FString& ServerAddress, const uint16 ServerPort)
	: BufferCapacity{BufferCapacity},
	  clientSocket_{INVALID_SOCKET},
	  serverAddress_{TCHAR_TO_UTF8(*ServerAddress)},
	  serverPort_{ServerPort}
{
}

bool FNetworkWorker::Init()
{
	return true;
}

uint32 FNetworkWorker::Run()
{
	SOCKADDR_IN addrIn{};
	addrIn.sin_family = AF_INET;
	addrIn.sin_port = htons(static_cast<unsigned short>(serverPort_));
	if (int Result = inet_pton(AF_INET, serverAddress_.c_str(), &addrIn.sin_addr); Result != 1)
	{
		StoreStopped("서버 주소 문자열 해석에 실패하였습니다:", WSAGetLastError());
		return 1;
	}

	const SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		StoreStopped("클라이언트 소켓 생성에 실패하였습니다:", WSAGetLastError());
		return 1;
	}
	
	const int Result = connect(clientSocket_, reinterpret_cast<const sockaddr*>(&addrIn), sizeof(SOCKADDR_IN));

	if (Result == SOCKET_ERROR)
	{
		StoreStopped("서버 접속에 실패하였습니다:", WSAGetLastError());
		closesocket(clientSocket);
		return 1;
	}

	isConnected_.store(true, std::memory_order_release);

	sendBufferBegin_.store(0, std::memory_order_release);
	sendBufferEnd_.store(0, std::memory_order_release);
	sendBufferSize_.store(0, std::memory_order_release);
	
	receiveBufferBegin_.store(0, std::memory_order_release);
	receiveBufferBegin_.store(0, std::memory_order_release);
	receiveBufferBegin_.store(0, std::memory_order_release);
	
	return 0;
}

void FNetworkWorker::Stop()
{
	StoreStopped();
}

void FNetworkWorker::Exit()
{
	
}


