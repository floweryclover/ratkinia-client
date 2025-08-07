#include "NetworkWorker.h"
#include "Ratkinia/Ratkinia.h"

namespace OpenSSL
{
#include "openssl/err.h"
}

#include <WS2tcpip.h>

FRatkiniaClientInitResult Initialize(const char* const ServerAddress, const uint16_t ServerPort)
{
	auto MakeInvalidResult = [](std::string Message)
	{
		return FRatkiniaClientInitResult
		{
			INVALID_SOCKET,
			nullptr,
			nullptr,
			{},
			std::move(Message)
		};
	};

	SOCKADDR_IN AddrIn;
	AddrIn.sin_family = AF_INET;
	AddrIn.sin_port = htons(ServerPort);
	if (const int Result = inet_pton(AF_INET, ServerAddress, &AddrIn.sin_addr); Result != 1)
	{
		return MakeInvalidResult(std::format("서버 주소 문자열 해석에 실패하였습니다: {}.", WSAGetLastError()));
	}

	const auto Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		return MakeInvalidResult(std::format("클라이언트 소켓 생성에 실패하였습니다: {}.", WSAGetLastError()));
	}

	constexpr int NoDelay = 1;
	if (SOCKET_ERROR == setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&NoDelay),
	                               sizeof(NoDelay)))
	{
		closesocket(Socket);
		return MakeInvalidResult(std::format("TCP_NODELAY 설정에 실패하였습니다: {}.", WSAGetLastError()));
	}

	const auto SslCtx = OpenSSL::SSL_CTX_new(OpenSSL::TLS_client_method());
	if (!SslCtx)
	{
		closesocket(Socket);
		char Buf[256];
		return MakeInvalidResult(std::format("SSL Context 생성에 실패하였습니다: {}.",
		                                     OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf)));
	}

	const auto Ssl = OpenSSL::SSL_new(SslCtx);
	if (!Ssl)
	{
		closesocket(Socket);
		OpenSSL::SSL_CTX_free(SslCtx);
		char Buf[256];
		return MakeInvalidResult(std::format("SSL 객체 생성에 실패하였습니다: {}.",
		                                     OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf)));
	}

	if (OpenSSL::SSL_set_fd(Ssl, Socket) == 0)
	{
		closesocket(Socket);
		OpenSSL::SSL_CTX_free(SslCtx);
		OpenSSL::SSL_free(Ssl);
		char Buf[256];
		return MakeInvalidResult(std::format("SSL 초기 설정에 실패하였습니다: {}.",
		                                     OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf)));
	}

	return
	{
		Socket,
		SslCtx,
		Ssl,
		AddrIn,
		""
	};
}

FNetworkWorker::FNetworkWorker(const FString& ServerAddress, const uint16 ServerPort)
	: FNetworkWorker(Initialize(TCHAR_TO_UTF8(*ServerAddress), ServerPort))
{
}

FNetworkWorker::FNetworkWorker(const FRatkiniaClientInitResult& InitResult)
	: SslCtx{InitResult.SslCtx},
	  Ssl{InitResult.Ssl},
	  ClientSocket{InitResult.Socket},
	  ServerAddrIn{InitResult.ServerAddrIn},
	  ConnectionState{ERatkiniaConnectionState::Connecting},
	  SendBuffer{std::make_unique<char[]>(BufferCapacity)},
	  SendContiguousPushBuffer{std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize)},
	  SendBufferHead{0},
	  SendBufferTail{0},
	  ReceiveBuffer{std::make_unique<char[]>(BufferCapacity)},
	  ReceiveContiguousPopBuffer{std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize)}
{
	if (ClientSocket == INVALID_SOCKET)
	{
		ConnectionState = ERatkiniaConnectionState::Disconnected;
		return;
	}

	AsyncTask(ENamedThreads::Type::AnyThread,
	          [this]
	          {
		          if (const int Result = connect(ClientSocket, reinterpret_cast<const sockaddr*>(&ServerAddrIn),
		                                         sizeof(SOCKADDR_IN));
			          Result == SOCKET_ERROR)
		          {
			          std::string Reason = std::format("서버 접속에 실패하였습니다: {}.",
			                                           InterpretWsaErrorCodeIfWellKnown(WSAGetLastError()));
			          Disconnect(std::move(Reason));
			          return;
		          }

		          if (OpenSSL::SSL_connect(Ssl) != 1)
		          {
			          char Buf[256];
			          std::string Reason = std::format("SSL 핸드셰이크에 실패하였습니다: {}.",
			                                           OpenSSL::ERR_error_string(
				                                           OpenSSL::ERR_get_error(), Buf));
			          Disconnect(std::move(Reason));
			          return;
		          }

	          	u_long NonBlock = 1;
				if (SOCKET_ERROR == ioctlsocket(ClientSocket, FIONBIO, &NonBlock))
				{
					std::string Reason = std::format("소켓 설정에 실패하였습니다: {}.", WSAGetLastError());
					Disconnect(std::move(Reason));
					return;
				}
	          	
		          ConnectionState = ERatkiniaConnectionState::Connected;
		          IoThread = std::thread{&FNetworkWorker::IoThreadBody, this};
	          });
}

FNetworkWorker::~FNetworkWorker()
{
	SSL_shutdown(Ssl);
	if (IoThread.joinable())
	{
		IoThread.join();
	}
	SSL_free(Ssl);
	SSL_CTX_free(SslCtx);
	shutdown(ClientSocket, SD_BOTH);
	closesocket(ClientSocket);
}

const std::string& FNetworkWorker::GetDisconnectedReason()
{
	std::lock_guard Lock{DisconnectedReasonMutex};
	return DisconnectedReason;
}

void FNetworkWorker::Disconnect(std::string Reason)
{
	if (ConnectionState.exchange(ERatkiniaConnectionState::Disconnected, std::memory_order_acq_rel) !=
		ERatkiniaConnectionState::Disconnected)
	{
		std::lock_guard Lock{DisconnectedReasonMutex};
		DisconnectedReason = std::move(Reason);
	}
}

TOptional<TScopedNetworkMessage<FNetworkWorker>> FNetworkWorker::TryPopMessage()
{
	using namespace RatkiniaProtocol;

	const auto loadedReceiveBufferHead{ReceiveBufferHead.load(std::memory_order_acquire)};
	const auto loadedReceiveBufferTail{ReceiveBufferTail.load(std::memory_order_acquire)};
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
	memcpy(&header, ReceiveBuffer.get() + loadedReceiveBufferHead, primaryHeaderSize);
	memcpy(reinterpret_cast<char*>(&header) + primaryHeaderSize, ReceiveBuffer.get(),
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
			this, 0, header.MessageType, header.BodyLength,
			ReceiveBuffer.get() + loadedReceiveBufferHeadAfterHeader
		};
	}

	memcpy(ReceiveContiguousPopBuffer.get(), ReceiveBuffer.get() + loadedReceiveBufferHeadAfterHeader,
	       primaryBodySize);
	memcpy(ReceiveContiguousPopBuffer.get() + primaryBodySize, ReceiveBuffer.get(), secondaryBodySize);

	return TScopedNetworkMessage{
		this, 0, header.MessageType, header.BodyLength,
		ReceiveContiguousPopBuffer.get()
	};
}

void FNetworkWorker::IoThreadBody()
{
	constexpr TIMEVAL TimeOut{0, 2'000};

	bool bWantWrite = false;
	while (true)
	{
		FD_SET FdReads;
		FD_SET FdWrites;
		FD_ZERO(&FdReads);
		FD_ZERO(&FdWrites);
		FD_SET(ClientSocket, &FdReads);
		
		const size_t LoadedSendBufferTail = SendBufferTail.load(std::memory_order_acquire);
		const size_t LoadedSendBufferHead = SendBufferHead.load(std::memory_order_relaxed);
		if (bWantWrite || LoadedSendBufferTail != LoadedSendBufferHead)
		{
			FD_SET(ClientSocket, &FdWrites);
		}
		bWantWrite = false;

		const int SelectResult = select(0, &FdReads, &FdWrites, nullptr, &TimeOut);
		if (SelectResult == 0)
		{
			continue;
		}
		if (SelectResult == SOCKET_ERROR)
		{
			Disconnect(std::format("송수신 스레드에서 에러가 발생하였습니다: {}.", WSAGetLastError()));
			break;
		}

		if (FD_ISSET(ClientSocket, &FdReads))
		{
			const size_t LoadedReceiveBufferTail = ReceiveBufferTail.load(std::memory_order_relaxed);
			const size_t LoadedReceiveBufferHead = ReceiveBufferHead.load(std::memory_order_acquire);
			const size_t ReceiveBufferSize =
				LoadedReceiveBufferTail >= LoadedReceiveBufferHead
					? LoadedReceiveBufferTail - LoadedReceiveBufferHead
					: LoadedReceiveBufferTail + BufferCapacity - LoadedReceiveBufferHead
			;
			const size_t ReceiveBufferAvailable = BufferCapacity - ReceiveBufferSize - 1;
			if (ReceiveBufferAvailable == 0)
			{
				continue;
			}

			const int ReadResult = OpenSSL::SSL_read(Ssl, ReceiveBuffer.get() + LoadedReceiveBufferTail,
												 std::min<size_t>(ReceiveBufferAvailable,
																  BufferCapacity - LoadedReceiveBufferTail));
			if (ReadResult <= 0)
			{
				const int SslError = OpenSSL::SSL_get_error(Ssl, ReadResult);
				if (SslError == SSL_ERROR_ZERO_RETURN)
				{
					Disconnect("서버와의 연결이 종료되었습니다.");
					break;
				}
				if (SslError == SSL_ERROR_WANT_WRITE)
				{
					bWantWrite = true;
					continue;
				}
				if (SslError == SSL_ERROR_WANT_READ)
				{
					continue;
				}

				char Buf[256];
				std::string Reason =
					std::format("네트워크 데이터 수신 중 에러가 발생하였습니다: SSL ERROR {}, {}.", SslError,
								OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf));
				Disconnect(std::move(Reason));
				break;
			}

			ReceiveBufferTail.store((LoadedReceiveBufferTail + ReadResult) % BufferCapacity, std::memory_order_release);
		}

		if (FD_ISSET(ClientSocket, &FdWrites))
		{
			const size_t Writable = LoadedSendBufferHead <= LoadedSendBufferTail ? LoadedSendBufferTail - LoadedSendBufferHead : BufferCapacity - LoadedSendBufferHead;
			const int WriteResult = OpenSSL::SSL_write(Ssl, SendBuffer.get() + LoadedSendBufferHead, Writable);

			if (WriteResult <= 0)
			{
				const int SslError = OpenSSL::SSL_get_error(Ssl, WriteResult);
				if (SslError == SSL_ERROR_ZERO_RETURN)
				{
					Disconnect("서버와의 연결이 종료되었습니다.");
					break;
				}
				if (SslError == SSL_ERROR_WANT_READ)
				{
					continue;
				}

				char Buf[256];
				std::string Reason =
					std::format("네트워크 데이터 송신 중 에러가 발생하였습니다: SSL ERROR {}, {}.", SslError,
								OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf));
				Disconnect(std::move(Reason));
				break;
			}

			SendBufferHead.store((LoadedSendBufferHead + WriteResult) % BufferCapacity, std::memory_order_release);
		}
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
