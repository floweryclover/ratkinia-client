#include "NetworkWorker.h"

namespace OpenSSL
{
#include "openssl/ssl.h"
#include "openssl/err.h"
}

#include <WinSock2.h>
#include <WS2tcpip.h>

FRatkiniaClientInitResult Initialize(const FString& ServerAddress, const uint16 ServerPort)
{
	auto MakeInvalidResult = [](FString Message)
	{
		return FRatkiniaClientInitResult
		{
			INVALID_SOCKET,
			nullptr,
			nullptr,
			{},
			MoveTemp(Message)
		};
	};

	SOCKADDR_IN AddrIn;
	AddrIn.sin_family = AF_INET;
	AddrIn.sin_port = htons(ServerPort);
	if (const int Result = inet_pton(AF_INET, TCHAR_TO_UTF8(*ServerAddress), &AddrIn.sin_addr); Result != 1)
	{
		return MakeInvalidResult(FString::Printf(TEXT("서버 주소 문자열 해석에 실패하였습니다: %d."), WSAGetLastError()));
	}

	const auto Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		return MakeInvalidResult(FString::Printf(TEXT("클라이언트 소켓 생성에 실패하였습니다: %d."), WSAGetLastError()));
	}

	constexpr int NoDelay = 1;
	if (SOCKET_ERROR == setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&NoDelay),
	                               sizeof(NoDelay)))
	{
		closesocket(Socket);
		return MakeInvalidResult(FString::Printf(TEXT("TCP_NODELAY 설정에 실패하였습니다: %d."), WSAGetLastError()));
	}

	const auto SslCtx = OpenSSL::SSL_CTX_new(OpenSSL::TLS_client_method());
	if (!SslCtx)
	{
		closesocket(Socket);
		char Buf[256];
		return MakeInvalidResult(FString::Printf(
			TEXT("SSL Context 생성에 실패하였습니다: %s."),
			UTF8_TO_TCHAR(OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf))));
	}

	const auto Ssl = OpenSSL::SSL_new(SslCtx);
	if (!Ssl)
	{
		closesocket(Socket);
		OpenSSL::SSL_CTX_free(SslCtx);
		char Buf[256];
		return MakeInvalidResult(FString::Printf(TEXT("SSL 객체 생성에 실패하였습니다: %s."),
		                                         UTF8_TO_TCHAR(
			                                         OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf))));
	}

	if (OpenSSL::SSL_set_fd(Ssl, Socket) == 0)
	{
		closesocket(Socket);
		OpenSSL::SSL_CTX_free(SslCtx);
		OpenSSL::SSL_free(Ssl);
		char Buf[256];
		return MakeInvalidResult(FString::Printf(TEXT("SSL 초기 설정에 실패하였습니다: %s."),
		                                         UTF8_TO_TCHAR(
			                                         OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf))));
	}

	FRatkiniaClientInitResult Result;
	Result.Socket = Socket;
	Result.SslCtx = SslCtx;
	Result.Ssl = Ssl;
	memcpy(Result.ServerAddrIn.GetData(), &AddrIn, sizeof(SOCKADDR_IN));
	return Result;
}

FNetworkWorker::FNetworkWorker(const FString& ServerAddress, const uint16 ServerPort)
	: FNetworkWorker(Initialize(TCHAR_TO_UTF8(*ServerAddress), ServerPort))
{
}

FNetworkWorker::FNetworkWorker(const FRatkiniaClientInitResult& InitResult)
	: SslCtx{InitResult.SslCtx},
	  Ssl{InitResult.Ssl},
	  Socket{InitResult.Socket},
	  ServerAddrIn{InitResult.ServerAddrIn},
	  IoThread{nullptr},
	  ConnectionState{ERatkiniaConnectionState::Connecting},
	  SendBuffer{MakeUnique<char[]>(BufferCapacity)},
	  SendContiguousPushBuffer{MakeUnique<char[]>(RatkiniaProtocol::MessageMaxSize)},
	  SendBufferHead{0},
	  SendBufferTail{0},
	  ReceiveBuffer{MakeUnique<char[]>(BufferCapacity)},
	  ReceiveContiguousPopBuffer{MakeUnique<char[]>(RatkiniaProtocol::MessageMaxSize)}
{
	if (Socket == INVALID_SOCKET)
	{
		ConnectionState = ERatkiniaConnectionState::Disconnected;
		return;
	}

	AsyncTask(ENamedThreads::Type::AnyThread,
	          [this]
	          {
		          if (const int Result = connect(Socket, reinterpret_cast<const sockaddr*>(&ServerAddrIn),
		                                         sizeof(SOCKADDR_IN));
			          Result == SOCKET_ERROR)
		          {
			          FString Reason = FString::Printf(
				          TEXT("서버 접속에 실패하였습니다: %s."), *InterpretWsaErrorCodeIfWellKnown(WSAGetLastError()));
			          Disconnect(MoveTemp(Reason));
			          return;
		          }

		          if (OpenSSL::SSL_connect(Ssl) != 1)
		          {
			          char Buf[256];
			          FString Reason = FString::Printf(TEXT("SSL 핸드셰이크에 실패하였습니다: %s."),
			                                           UTF8_TO_TCHAR(OpenSSL::ERR_error_string(
				                                           OpenSSL::ERR_get_error(), Buf)));
			          Disconnect(MoveTemp(Reason));
			          return;
		          }

		          u_long NonBlock = 1;
		          if (SOCKET_ERROR == ioctlsocket(Socket, FIONBIO, &NonBlock))
		          {
			          FString Reason = FString::Printf(TEXT("소켓 설정에 실패하였습니다: %d."), WSAGetLastError());
			          Disconnect(MoveTemp(Reason));
			          return;
		          }
		          ConnectionState = ERatkiniaConnectionState::Connected;
		          IoThread = FRunnableThread::Create(this, TEXT("NetworkThread"));
	          });
}

FNetworkWorker::~FNetworkWorker()
{
	shutdown(Socket, SD_BOTH);
	IoThread->WaitForCompletion();
	SSL_shutdown(Ssl);
	SSL_free(Ssl);
	SSL_CTX_free(SslCtx);
	closesocket(Socket);
}

const FString& FNetworkWorker::GetDisconnectedReason()
{
	FScopeLock Lock{&DisconnectedReasonMutex};
	return DisconnectedReason;
}

void FNetworkWorker::Disconnect(FString Reason)
{
	if (ConnectionState.exchange(ERatkiniaConnectionState::Disconnected, std::memory_order_acq_rel) !=
		ERatkiniaConnectionState::Disconnected)
	{
		FScopeLock Lock{&DisconnectedReasonMutex};
		DisconnectedReason = MoveTemp(Reason);
	}
}

TOptional<FMessagePeekResult> FNetworkWorker::TryPeekMessage()
{
	using namespace RatkiniaProtocol;

	const auto LoadedHead = ReceiveBufferHead.load(std::memory_order_acquire);
	const auto LoadedTail = ReceiveBufferTail.load(std::memory_order_acquire);
	const auto Size =
		LoadedTail >= LoadedHead
			? LoadedTail - LoadedHead
			: LoadedTail + BufferCapacity - LoadedHead;

	if (Size < MessageHeaderSize)
	{
		return NullOpt;
	}

	const auto PrimaryHeaderSize = std::min<size_t>(MessageHeaderSize, BufferCapacity - LoadedHead);
	const auto SecondaryHeaderSize = MessageHeaderSize - PrimaryHeaderSize;
	FMessageHeader Header{};
	memcpy(&Header, ReceiveBuffer.Get() + LoadedHead, PrimaryHeaderSize);
	memcpy(reinterpret_cast<char*>(&Header) + PrimaryHeaderSize, ReceiveBuffer.Get(),
	       SecondaryHeaderSize);
	Header.MessageType = ntohs(Header.MessageType);
	Header.BodySize = ntohs(Header.BodySize);

	const auto MessageTotalSize = MessageHeaderSize + Header.BodySize;
	if (Size < MessageTotalSize)
	{
		return NullOpt;
	}

	const auto LoadedHeadAfterHeader = (LoadedHead + MessageHeaderSize) % BufferCapacity;
	const auto PrimaryBodySize = std::min<size_t>(Header.BodySize,
	                                              BufferCapacity - LoadedHeadAfterHeader);
	const auto SecondaryBodySize = Header.BodySize - PrimaryBodySize;

	if (SecondaryBodySize == 0)
	{
		return FMessagePeekResult
		{
			Header.MessageType,
			Header.BodySize,
			ReceiveBuffer.Get() + LoadedHeadAfterHeader
		};
	}

	memcpy(ReceiveContiguousPopBuffer.Get(), ReceiveBuffer.Get() + LoadedHeadAfterHeader,
	       PrimaryBodySize);
	memcpy(ReceiveContiguousPopBuffer.Get() + PrimaryBodySize, ReceiveBuffer.Get(), SecondaryBodySize);

	return FMessagePeekResult
	{
		Header.MessageType,
		Header.BodySize,
		ReceiveContiguousPopBuffer.Get()
	};
}

uint32 FNetworkWorker::Run()
{
	constexpr TIMEVAL TimeOut{0, 2'000};

	bool bWantWrite = false;
	while (ConnectionState.load(std::memory_order_relaxed) == ERatkiniaConnectionState::Connected)
	{
		FD_SET FdReads;
		FD_SET FdWrites;
		FD_ZERO(&FdReads);
		FD_ZERO(&FdWrites);
		FD_SET(Socket, &FdReads);

		const size_t LoadedSendBufferTail = SendBufferTail.load(std::memory_order_acquire);
		const size_t LoadedSendBufferHead = SendBufferHead.load(std::memory_order_relaxed);
		if (bWantWrite || LoadedSendBufferTail != LoadedSendBufferHead)
		{
			FD_SET(Socket, &FdWrites);
		}
		bWantWrite = false;

		const int SelectResult = select(0, &FdReads, &FdWrites, nullptr, &TimeOut);
		if (SelectResult == 0)
		{
			continue;
		}
		if (SelectResult == SOCKET_ERROR)
		{
			Disconnect(FString::Printf(
				TEXT("네트워크 스레드에서 에러가 발생하였습니다: %s."), *InterpretWsaErrorCodeIfWellKnown(WSAGetLastError())));
			break;
		}

		if (FD_ISSET(Socket, &FdReads))
		{
			const size_t LoadedReceiveBufferTail = ReceiveBufferTail.load(std::memory_order_relaxed);
			const size_t LoadedReceiveBufferHead = ReceiveBufferHead.load(std::memory_order_acquire);
			const size_t ReceiveBufferSize =
				LoadedReceiveBufferTail >= LoadedReceiveBufferHead
					? LoadedReceiveBufferTail - LoadedReceiveBufferHead
					: LoadedReceiveBufferTail + BufferCapacity - LoadedReceiveBufferHead;
			const size_t ReceiveBufferAvailable = BufferCapacity - ReceiveBufferSize - 1;
			if (ReceiveBufferAvailable == 0)
			{
				continue;
			}

			const int ReadResult = OpenSSL::SSL_read(Ssl, ReceiveBuffer.Get() + LoadedReceiveBufferTail,
			                                         std::min<size_t>(ReceiveBufferAvailable,
			                                                          BufferCapacity - LoadedReceiveBufferTail));
			if (ReadResult <= 0)
			{
				const int SslError = OpenSSL::SSL_get_error(Ssl, ReadResult);
				if (ReadResult == 0 || SslError == SSL_ERROR_ZERO_RETURN)
				{
					Disconnect(TEXT("서버와의 연결이 종료되었습니다."));
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
				FString Reason =
					FString::Printf(TEXT("네트워크 데이터 수신 중 에러가 발생하였습니다: SSL ERROR %d, %s."), SslError,
					                UTF8_TO_TCHAR(OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf)));
				Disconnect(MoveTemp(Reason));
				break;
			}

			ReceiveBufferTail.store((LoadedReceiveBufferTail + ReadResult) % BufferCapacity, std::memory_order_release);
		}

		if (FD_ISSET(Socket, &FdWrites))
		{
			const size_t Writable = LoadedSendBufferHead <= LoadedSendBufferTail
				                        ? LoadedSendBufferTail - LoadedSendBufferHead
				                        : BufferCapacity - LoadedSendBufferHead;
			const int WriteResult = OpenSSL::SSL_write(Ssl, SendBuffer.Get() + LoadedSendBufferHead, Writable);

			if (WriteResult <= 0)
			{
				const int SslError = OpenSSL::SSL_get_error(Ssl, WriteResult);
				if (SslError == SSL_ERROR_ZERO_RETURN)
				{
					Disconnect(TEXT("서버와의 연결이 종료되었습니다."));
					break;
				}
				if (SslError == SSL_ERROR_WANT_READ)
				{
					continue;
				}

				char Buf[256];
				FString Reason =
					FString::Printf(
						TEXT("네트워크 데이터 송신 중 에러가 발생하였습니다: SSL ERROR %d, %s."), SslError,
						UTF8_TO_TCHAR(OpenSSL::ERR_error_string(OpenSSL::ERR_get_error(), Buf)));

				Disconnect(MoveTemp(Reason));
				break;
			}

			SendBufferHead.store((LoadedSendBufferHead + WriteResult) % BufferCapacity, std::memory_order_release);
		}
	}

	return 0;
}

FString InterpretWsaErrorCodeIfWellKnown(const int32 ErrorCode)
{
	if (ErrorCode == WSAECONNRESET || ErrorCode == WSAECONNABORTED)
	{
		return TEXT("서버 측에서 연결을 종료했습니다");
	}

	if (ErrorCode == WSAECONNREFUSED)
	{
		return TEXT("서버에 연결할 수 없습니다");
	}

	return FString::Printf(TEXT("에러 코드 %d"), ErrorCode);
}
