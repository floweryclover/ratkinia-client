#include "NetworkWorker.h"
#include <WS2tcpip.h>

#include "Ratkinia/Ratkinia.h"

FNetworkWorker::FNetworkWorker(const uint64 BufferCapacity)
	: BufferCapacity{BufferCapacity},
	  sendBuffer_{std::make_unique<char[]>(BufferCapacity)},
	  receiveBuffer_{std::make_unique<char[]>(BufferCapacity)},
	  receiveTempBuffer_{std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize)},
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
	if (SOCKET_ERROR == setsockopt(clientSocket_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&option),
	                               sizeof(option)))
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

		          receiveBufferBegin_.store(0, std::memory_order_release);
		          receiveBufferEnd_.store(0, std::memory_order_release);

		          sendThread_ = std::thread{&FNetworkWorker::SendThreadBody, this};
		          receiveThread_ = std::thread{&FNetworkWorker::ReceiveThreadBody, this};
	          });
}

TOptional<TScopedNetworkMessage<FNetworkWorker>> FNetworkWorker::TryPopMessage()
{
	using namespace RatkiniaProtocol;

	const auto loadedReceiveBufferHead{receiveBufferBegin_.load(std::memory_order_acquire)};
	const auto loadedReceiveBufferTail{receiveBufferEnd_.load(std::memory_order_acquire)};
	const auto receiveBufferSize{
		loadedReceiveBufferTail >= loadedReceiveBufferHead
			? loadedReceiveBufferTail - loadedReceiveBufferHead
			: loadedReceiveBufferTail + BufferCapacity - loadedReceiveBufferHead
	};
	if (receiveBufferSize < MessageHeaderSize)
	{
		return NullOpt;
	}

	const auto primaryHeaderSize = std::min<size_t>(MessageHeaderSize, BufferCapacity - loadedReceiveBufferHead);
	const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;
	MessageHeader header{};
	memcpy_s(&header, primaryHeaderSize, receiveBuffer_.get() + loadedReceiveBufferHead, primaryHeaderSize);
	memcpy_s(reinterpret_cast<char*>(&header) + primaryHeaderSize, secondaryHeaderSize, receiveBuffer_.get(),
	         secondaryHeaderSize);
	header.MessageType = ntohs(header.MessageType);
	header.BodyLength = ntohs(header.BodyLength);

	const auto messageTotalSize = MessageHeaderSize + header.BodyLength;
	if (receiveBufferSize < messageTotalSize)
	{
		return NullOpt;
	}

	const auto loadedReceiveBufferHeadAfterHeader{(loadedReceiveBufferHead + MessageHeaderSize) % BufferCapacity};
	const auto primaryBodySize = std::min<size_t>(header.BodyLength,
	                                              BufferCapacity - loadedReceiveBufferHeadAfterHeader);
	const auto secondaryBodySize = header.BodyLength - primaryBodySize;

	if (secondaryBodySize == 0)
	{
		return TScopedNetworkMessage{
			this, 0, header.MessageType, header.BodyLength, receiveBuffer_.get() + loadedReceiveBufferHeadAfterHeader
		};
	}

	memcpy(receiveTempBuffer_.get(), receiveBuffer_.get() + loadedReceiveBufferHeadAfterHeader, primaryBodySize);
	memcpy(receiveTempBuffer_.get() + primaryBodySize, receiveBuffer_.get(), secondaryBodySize);

	return TScopedNetworkMessage{
		this, 0, header.MessageType, header.BodyLength,
		receiveTempBuffer_.get()
	};
}

void FNetworkWorker::SendThreadBody()
{
	while (!isStopped_.load(std::memory_order_acquire))
	{
		const auto loadedSendBufferHead{sendBufferBegin_.load(std::memory_order_acquire)};
		const auto loadedSendBufferTail{sendBufferEnd_.load(std::memory_order_acquire)};
		const auto sendBufferSize{
			loadedSendBufferTail >= loadedSendBufferHead
				? loadedSendBufferTail - loadedSendBufferHead
				: loadedSendBufferTail + BufferCapacity - loadedSendBufferHead
		};
		if (sendBufferSize == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		const auto result = send(clientSocket_, sendBuffer_.get() + loadedSendBufferHead,
		                         std::min<size_t>(sendBufferSize, BufferCapacity - loadedSendBufferHead), 0);
		if (result <= 0)
		{
			if (result < 0)
			{
				StoreStopped("수신 중 에러가 발생하였습니다:", WSAGetLastError());
			}
			else
			{
				StoreStopped("서버와의 연결이 종료되었습니다.");
			}
			break;
		}
		sendBufferBegin_.store((loadedSendBufferHead + result) % BufferCapacity, std::memory_order_release);
	}
}

void FNetworkWorker::ReceiveThreadBody()
{
	while (!isStopped_.load(std::memory_order_acquire))
	{
		const auto loadedReceiveBufferHead{receiveBufferBegin_.load(std::memory_order_acquire)};
		const auto loadedReceiveBufferTail{receiveBufferEnd_.load(std::memory_order_acquire)};
		const auto receiveBufferSize{
			loadedReceiveBufferTail >= loadedReceiveBufferHead
				? loadedReceiveBufferTail - loadedReceiveBufferHead
				: loadedReceiveBufferTail + BufferCapacity - loadedReceiveBufferHead
		};
		const auto receiveBufferAvailable{BufferCapacity - receiveBufferSize - 1};

		if (receiveBufferAvailable == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
			continue;
		}

		const auto result = recv(clientSocket_, receiveBuffer_.get() + loadedReceiveBufferTail,
		                         std::min<size_t>(receiveBufferAvailable,
		                                          BufferCapacity - loadedReceiveBufferTail), 0);
		if (result <= 0)
		{
			if (result < 0)
			{
				StoreStopped("송신 중 에러가 발생하였습니다:", WSAGetLastError());
			}
			else
			{
				StoreStopped("서버와의 연결이 종료되었습니다.");
			}
			break;
		}

		receiveBufferEnd_.store((loadedReceiveBufferTail + result) % BufferCapacity, std::memory_order_release);
	}
}
