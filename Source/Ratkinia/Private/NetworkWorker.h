#pragma once

#include "RatkiniaProtocol.gen.h"
#include "ScopedNetworkMessage.h"
namespace OpenSSL
{
#include "openssl/ssl.h"
}
#include <WinSock2.h>
#include <mutex>
#include <format>

struct FRatkiniaClientInitResult
{
	SOCKET Socket;
	OpenSSL::SSL_CTX* SslCtx;
	OpenSSL::SSL* Ssl;
	SOCKADDR_IN ServerAddrIn;
	std::string FailedReason;
};

FRatkiniaClientInitResult Initialize(const char* ServerAddress, uint16_t ServerPort);

enum class ERatkiniaConnectionState : uint8_t
{
	NotConnected,
	Connecting,
	Connected,
	Disconnected
};

std::string InterpretWsaErrorCodeIfWellKnown(int errorCode);

class alignas(64) FNetworkWorker final
{
public:
	static constexpr size_t BufferCapacity{65536};

	explicit FNetworkWorker(const FString& ServerAddress, const uint16 ServerPort);

	~FNetworkWorker();

	const std::string& GetDisconnectedReason();
	
	void Disconnect(std::string Reason);
	
	FORCEINLINE ERatkiniaConnectionState GetConnectionState()
	{
		return ConnectionState.load(std::memory_order_relaxed);
	}

	FORCEINLINE void ReleaseScopedNetworkMessage(const TScopedNetworkMessage<FNetworkWorker>& message)
	{
		ReceiveBufferHead.store(
			(ReceiveBufferHead.load(std::memory_order_acquire) + RatkiniaProtocol::MessageHeaderSize + message.
				GetBodySize()) % BufferCapacity,
			std::memory_order_release);
	}
	
	template <typename TMessage>
	void Send(TMessage&& Message, const RatkiniaProtocol::CtsMessageType MessageType)
	{
		using namespace RatkiniaProtocol;

		check(IsInGameThread());

		const size_t MessageBodySize = Message.ByteSizeLong();
		const size_t MessageTotalSize = MessageBodySize + MessageHeaderSize;
		if (MessageTotalSize > MessageMaxSize)
		{
			Disconnect(std::format("메시지 크기가 허용 범위를 초과했습니다: {}/{}.", MessageTotalSize, MessageMaxSize));
			return;
		}

		const size_t LoadedSendBufferTail = SendBufferTail.load(std::memory_order_relaxed);
		const size_t LoadedSendBufferHead = SendBufferHead.load(std::memory_order_acquire);
		
		const size_t SendBufferSize =
			LoadedSendBufferTail >= LoadedSendBufferHead
				? LoadedSendBufferTail - LoadedSendBufferHead
				: LoadedSendBufferTail + BufferCapacity - LoadedSendBufferHead;
		
		
		if (const size_t SendBufferAvailableSize = BufferCapacity - SendBufferSize - 1;
			SendBufferAvailableSize < MessageTotalSize)
		{
			Disconnect(std::format("송신 버퍼에 여유 공간이 없습니다: SendBuffer: {}/{}, MessageTotalSize: {}.", SendBufferAvailableSize, BufferCapacity-1, MessageTotalSize));
			return;
		}

		const MessageHeader Header
		{
			htons(static_cast<uint16_t>(MessageType)),
			htons(MessageBodySize)
		};

		const size_t PrimaryHeaderSize = std::min<
			size_t>(MessageHeaderSize, BufferCapacity - LoadedSendBufferTail);
		const size_t SecondaryHeaderSize = MessageHeaderSize - PrimaryHeaderSize;

		memcpy(SendBuffer.get() + LoadedSendBufferTail, &Header, PrimaryHeaderSize);
		memcpy(SendBuffer.get(), reinterpret_cast<const char*>(&Header) + PrimaryHeaderSize, SecondaryHeaderSize);

		const size_t TailAfterHeader = (LoadedSendBufferTail + MessageHeaderSize) % BufferCapacity;
		const size_t PrimaryBodySize = std::min<size_t>(MessageBodySize, BufferCapacity - TailAfterHeader);
		const size_t SecondaryBodySize = MessageBodySize - PrimaryBodySize;

		if (PrimaryBodySize == MessageBodySize)
		{
			Message.SerializeToArray(SendBuffer.get() + TailAfterHeader, MessageBodySize);
		}
		else
		{
			Message.SerializeToArray(SendContiguousPushBuffer.get(), MessageTotalSize);
			memcpy(SendBuffer.get() + TailAfterHeader, SendContiguousPushBuffer.get(), PrimaryBodySize);
			memcpy(SendBuffer.get(), SendContiguousPushBuffer.get() + PrimaryBodySize, SecondaryBodySize);
		}

		SendBufferTail.store((LoadedSendBufferTail + MessageTotalSize) % BufferCapacity, std::memory_order_release);
	}

	TOptional<TScopedNetworkMessage<FNetworkWorker>> TryPopMessage();

private:
	OpenSSL::SSL_CTX* const SslCtx;
	OpenSSL::SSL* const Ssl;
	const SOCKET ClientSocket;
	const SOCKADDR_IN ServerAddrIn;
	
	std::thread IoThread;
	std::atomic<ERatkiniaConnectionState> ConnectionState;

	std::mutex DisconnectedReasonMutex;
	std::string DisconnectedReason;
	
	alignas(64) const std::unique_ptr<char[]> SendBuffer;
	alignas(64) const std::unique_ptr<char[]> SendContiguousPushBuffer;
	alignas(64) std::atomic<size_t> SendBufferHead;
	alignas(64) std::atomic<size_t> SendBufferTail; // Exclusive
	
	alignas(64) const std::unique_ptr<char[]> ReceiveBuffer;
	alignas(64) const std::unique_ptr<char[]> ReceiveContiguousPopBuffer;
	alignas(64) std::atomic<size_t> ReceiveBufferHead;
	alignas(64) std::atomic<size_t> ReceiveBufferTail; // Exclusive

	explicit FNetworkWorker(const FRatkiniaClientInitResult& InitResult);
	
	void IoThreadBody();
};
