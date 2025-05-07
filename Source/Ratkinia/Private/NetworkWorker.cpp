#include "NetworkWorker.h"
#include <WS2tcpip.h>

#include "Ratkinia/Ratkinia.h"

FNetworkWorker::FNetworkWorker(const uint64 BufferCapacity)
	: BufferCapacity{BufferCapacity},
	  sendBuffer_{std::make_unique<char[]>(BufferCapacity)},
	  receiveBuffer_{std::make_unique<char[]>(BufferCapacity)},
	  clientSocket_{INVALID_SOCKET}
{
}

FNetworkWorker::~FNetworkWorker()
{
	isStopped_.store(true);
	if (clientSocket_ != INVALID_SOCKET)
	{
		shutdown(clientSocket_, SD_BOTH);
		closesocket(clientSocket_);
	}

	if (sendThread_.joinable())
	{
		sendThread_.join();
	}

	if (receiveThread_.joinable())
	{
		receiveThread_.join();
	}
}

void FNetworkWorker::Connect(const FString& ServerAddress, const uint16 ServerPort)
{
	if (clientSocket_ != INVALID_SOCKET)
	{
		StoreStopped("이미 접속 중입니다.");
		return;
	}

	SOCKADDR_IN addrIn{};
	addrIn.sin_family = AF_INET;
	addrIn.sin_port = htons(static_cast<unsigned short>(ServerPort));
	if (int Result = inet_pton(AF_INET, TCHAR_TO_UTF8(*ServerAddress), &addrIn.sin_addr); Result != 1)
	{
		StoreStopped("서버 주소 문자열 해석에 실패하였습니다:", WSAGetLastError());
		return;
	}

	clientSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket_ == INVALID_SOCKET)
	{
		StoreStopped("클라이언트 소켓 생성에 실패하였습니다:", WSAGetLastError());
		return;
	}

	const int option = 1;
	if (SOCKET_ERROR == setsockopt(clientSocket_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&option), sizeof(option)))
	{
		StoreStopped("TCP_NODELAY 설정에 실패하였습니다:", WSAGetLastError());
		return;
	}

	AsyncTask(ENamedThreads::Type::AnyThread,
	          [this, addrIn]
	          {
		          const int Result = connect(clientSocket_, reinterpret_cast<const sockaddr*>(&addrIn),
		                                     sizeof(SOCKADDR_IN));

		          if (Result == SOCKET_ERROR)
		          {
			          StoreStopped("서버 접속에 실패하였습니다:", WSAGetLastError());
			          closesocket(clientSocket_);
		          }

		          isConnected_.store(true, std::memory_order_release);

		          sendBufferBegin_.store(0, std::memory_order_release);
		          sendBufferEnd_.store(0, std::memory_order_release);
		          sendBufferSize_.store(0, std::memory_order_release);

		          receiveBufferBegin_.store(0, std::memory_order_release);
		          receiveBufferBegin_.store(0, std::memory_order_release);
		          receiveBufferBegin_.store(0, std::memory_order_release);

		          sendThread_ = std::thread{&FNetworkWorker::SendThreadBody, this};
		          receiveThread_ = std::thread{&FNetworkWorker::ReceiveThreadBody, this};
	          });
}

bool FNetworkWorker::TryPopMessage(uint16& OutMessageType, uint16& OutBodyLength, char* const OutMessageBodyBuffer,
                                   const int32 InMessageBodyBufferSize)
{
	using namespace RatkiniaProtocol;

	const auto receiveBufferSize = receiveBufferSize_.load(std::memory_order_acquire);
	if (receiveBufferSize < MessageHeaderSize)
	{
		return false;
	}

	MessageHeader header{};
	const auto receiveBufferBegin_beforeHeader = receiveBufferBegin_.load(std::memory_order_acquire);
	const auto primaryHeaderSize = std::min<
		size_t>(MessageHeaderSize, BufferCapacity - receiveBufferBegin_beforeHeader);
	const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

	memcpy_s(&header, primaryHeaderSize, receiveBuffer_.get() + receiveBufferBegin_beforeHeader, primaryHeaderSize);
	memcpy_s(reinterpret_cast<char*>(&header) + primaryHeaderSize, secondaryHeaderSize, receiveBuffer_.get(),
	         secondaryHeaderSize);
	header.MessageType = ntohs(header.MessageType);
	header.BodyLength = ntohs(header.BodyLength);

	const auto messageTotalSize = MessageHeaderSize + header.BodyLength;
	if (receiveBufferSize < messageTotalSize)
	{
		return false;
	}
	if (InMessageBodyBufferSize < header.BodyLength)
	{
		StoreStopped("InMessageBodyBufferSize <", header.BodyLength);
		return false;
	}

	OutMessageType = header.MessageType;
	OutBodyLength = header.BodyLength;

	const auto receiveBufferBegin_afterHeader = (receiveBufferBegin_beforeHeader + MessageHeaderSize) % BufferCapacity;
	const auto primaryBodySize = std::min<size_t>(header.BodyLength, BufferCapacity - receiveBufferBegin_afterHeader);
	const auto secondaryBodySize = header.BodyLength - primaryBodySize;

	if (primaryBodySize == header.BodyLength)
	{
		memcpy_s(OutMessageBodyBuffer, header.BodyLength, receiveBuffer_.get() + receiveBufferBegin_afterHeader,
		         header.BodyLength);
	}
	else
	{
		memcpy_s(OutMessageBodyBuffer, primaryBodySize, receiveBuffer_.get() + receiveBufferBegin_afterHeader,
		         primaryBodySize);
		memcpy_s(OutMessageBodyBuffer + primaryBodySize, secondaryBodySize, receiveBuffer_.get(), secondaryBodySize);
	}

	receiveBufferBegin_.store((receiveBufferBegin_afterHeader + header.BodyLength) % BufferCapacity,
	                          std::memory_order_release);
	receiveBufferSize_.fetch_sub(messageTotalSize, std::memory_order_release);
	return true;
}

void FNetworkWorker::SendThreadBody()
{
	while (!isStopped_.load(std::memory_order_acquire))
	{
		const auto sendBufferSize = sendBufferSize_.load(std::memory_order_acquire);
		if (sendBufferSize == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		const auto sendBufferBegin = sendBufferBegin_.load(std::memory_order_acquire);
		const auto result = send(clientSocket_, sendBuffer_.get() + sendBufferBegin,
		                         std::min<size_t>(sendBufferSize, BufferCapacity - sendBufferBegin), 0);
		if (result == 0)
		{
			StoreStopped("서버와의 연결이 종료되었습니다.");
			break;
		}
		else if (result < 0)
		{
			StoreStopped("수신 중 에러가 발생하였습니다:", WSAGetLastError());
			break;
		}
		UE_LOG(LogRatkinia, Log, TEXT("Sent %d"), result);
		sendBufferBegin_.store((sendBufferBegin + result) % BufferCapacity, std::memory_order_release);
		sendBufferSize_.fetch_sub(result, std::memory_order_release);
	}
}

void FNetworkWorker::ReceiveThreadBody()
{
	while (!isStopped_.load(std::memory_order_acquire))
	{
		const auto receiveBufferSize = receiveBufferSize_.load(std::memory_order_acquire);
		if (receiveBufferSize == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		const auto receiveBufferEnd = receiveBufferEnd_.load(std::memory_order_acquire);
		const auto result = recv(clientSocket_, receiveBuffer_.get() + receiveBufferEnd,
		                         std::min<size_t>(BufferCapacity - receiveBufferSize,
		                                          BufferCapacity - receiveBufferEnd), 0);
		if (result == 0)
		{
			StoreStopped("서버와의 연결이 종료되었습니다.");
			break;
		}
		else if (result < 0)
		{
			StoreStopped("송신 중 에러가 발생하였습니다:", WSAGetLastError());
			break;
		}

		receiveBufferEnd_.store((receiveBufferEnd + result) % BufferCapacity, std::memory_order_release);
		receiveBufferSize_.fetch_add(result, std::memory_order_release);
	}
}
