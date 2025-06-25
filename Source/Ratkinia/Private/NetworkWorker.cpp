#include "NetworkWorker.h"
#include <WS2tcpip.h>

#include "Ratkinia/Ratkinia.h"

FNetworkWorker::FNetworkWorker()
	: clientSocket_{INVALID_SOCKET},
	  connectionState_{ERatkiniaConnectionState::NotConnected},
	  sendBuffer_{std::make_unique<char[]>(BufferCapacity)},
	  receiveBuffer_{std::make_unique<char[]>(BufferCapacity)},
	  receiveTempBuffer_{std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize)}
{
}

FNetworkWorker::~FNetworkWorker()
{
	Cleanup();
	if (sendThread_.joinable())
	{
		sendThread_.join();
	}

	if (receiveThread_.joinable())
	{
		receiveThread_.join();
	}
}

void FNetworkWorker::Connect(const FString& serverAddress, const uint16 serverPort)
{
	check(connectionState_ == ERatkiniaConnectionState::NotConnected);

	SOCKADDR_IN addrIn{};
	addrIn.sin_family = AF_INET;
	addrIn.sin_port = htons(serverPort);
	if (const int result = inet_pton(AF_INET, TCHAR_TO_UTF8(*serverAddress), &addrIn.sin_addr); result != 1)
	{
		std::string reason{std::format("서버 주소 문자열 해석에 실패하였습니다: {}.", WSAGetLastError())};
		Disconnect(std::move(reason));
		return;
	}

	clientSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket_ == INVALID_SOCKET)
	{
		std::string reason{std::format("클라이언트 소켓 생성에 실패하였습니다: {}.", WSAGetLastError())};
		Disconnect(std::move(reason));
		return;
	}

	const int option = 1;
	if (SOCKET_ERROR == setsockopt(clientSocket_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&option),
	                               sizeof(option)))
	{
		std::string reason{std::format("TCP_NODELAY 설정에 실패하였습니다: {}.", WSAGetLastError())};
		Disconnect(std::move(reason));
		return;
	}

	connectionState_ = ERatkiniaConnectionState::Connecting;

	AsyncTask(ENamedThreads::Type::AnyThread,
	          [this, addrIn]
	          {
		          const int result = connect(clientSocket_, reinterpret_cast<const sockaddr*>(&addrIn),
		                                     sizeof(SOCKADDR_IN));

		          if (result == SOCKET_ERROR)
		          {
			          std::string reason{
				          std::format("서버 접속에 실패하였습니다: {}.", InterpretWsaErrorCodeIfWellKnown(WSAGetLastError()))
			          };

			          Disconnect(std::move(reason));;
			          return;
		          }

		          sendBufferBegin_.store(0, std::memory_order_release);
		          sendBufferEnd_.store(0, std::memory_order_release);

		          receiveBufferBegin_.store(0, std::memory_order_release);
		          receiveBufferEnd_.store(0, std::memory_order_release);

		          connectionState_ = ERatkiniaConnectionState::Connected;

		          sendThread_ = std::thread{&FNetworkWorker::SendThreadBody, this};
		          receiveThread_ = std::thread{&FNetworkWorker::ReceiveThreadBody, this};
	          });
}

const std::string& FNetworkWorker::GetDisconnectedReason()
{
	std::lock_guard lock{disconnectedReasonMutex_};
	return disconnectedReason_;
}

void FNetworkWorker::Disconnect(std::string reason)
{
	if (connectionState_.exchange(ERatkiniaConnectionState::Disconnected, std::memory_order_acq_rel) !=
		ERatkiniaConnectionState::Disconnected)
	{
		{
			std::lock_guard lock{disconnectedReasonMutex_};
			disconnectedReason_ = std::move(reason);
		}
		Cleanup();
	}
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
	UE_LOG(LogRatkinia, Log, TEXT("송신 스레드 시작"));

	while (true)
	{
		const auto loadedSendBufferHead{sendBufferBegin_.load(std::memory_order_acquire)};
		const auto loadedSendBufferTail{sendBufferEnd_.load(std::memory_order_acquire)};
		const auto sendBufferSize{
			loadedSendBufferTail >= loadedSendBufferHead
				? loadedSendBufferTail - loadedSendBufferHead
				: loadedSendBufferTail + BufferCapacity - loadedSendBufferHead
		};

		if (connectionState_.load(std::memory_order_acquire) != ERatkiniaConnectionState::Connected)
		{
			break;
		}

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
				std::string reason{std::format("네트워크 데이터 송신 중 에러가 발생하였습니다: {}.", InterpretWsaErrorCodeIfWellKnown(WSAGetLastError()))};
				Disconnect(std::move(reason));
			}
			else
			{
				std::string reason{"서버와의 연결이 종료되었습니다."};
				Disconnect(std::move(reason));
			}
			break;
		}
		sendBufferBegin_.store((loadedSendBufferHead + result) % BufferCapacity, std::memory_order_release);
	}

	UE_LOG(LogRatkinia, Log, TEXT("송신 스레드 종료"));
}

void FNetworkWorker::ReceiveThreadBody()
{
	UE_LOG(LogRatkinia, Log, TEXT("수신 스레드 시작"));

	while (true)
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
				std::string reason{std::format("네트워크 데이터 수신 중 에러가 발생하였습니다: {}.", InterpretWsaErrorCodeIfWellKnown(WSAGetLastError()))};
				Disconnect(std::move(reason));
			}
			else
			{
				std::string reason{"서버와의 연결이 종료되었습니다."};
				Disconnect(std::move(reason));
			}
			break;
		}

		receiveBufferEnd_.store((loadedReceiveBufferTail + result) % BufferCapacity, std::memory_order_release);
	}

	UE_LOG(LogRatkinia, Log, TEXT("수신 스레드 종료"));
}

void FNetworkWorker::Cleanup()
{
	if (clientSocket_ != INVALID_SOCKET)
	{
		shutdown(clientSocket_, SD_BOTH);
		closesocket(clientSocket_);
		clientSocket_ = INVALID_SOCKET;
	}
}

std::string InterpretWsaErrorCodeIfWellKnown(const int errorCode)
{
	if (errorCode == WSAECONNRESET || errorCode == WSAECONNABORTED)
	{
		return "서버 측에서 연결을 종료했습니다";
	}

	if (errorCode == WSAECONNREFUSED)
	{
		return "서버에 연결할 수 없습니다";
	}

	return std::format("에러 코드 {}", errorCode);
}
