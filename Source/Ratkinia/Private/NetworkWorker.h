#pragma once

#include "RatkiniaProtocol.gen.h"

namespace OpenSSL
{
	typedef struct ssl_ctx_st SSL_CTX;
	typedef struct ssl_st SSL;
}

typedef UINT_PTR SOCKET;

struct FRatkiniaClientInitResult
{
	SOCKET Socket;
	OpenSSL::SSL_CTX* SslCtx;
	OpenSSL::SSL* Ssl;
	TStaticArray<char, 16> ServerAddrIn;
	FString FailedReason;
};

inline uint16 Htons(const uint16 value)
{
	return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

FRatkiniaClientInitResult Initialize(const FString& ServerAddress, uint16 ServerPort);

enum class ERatkiniaConnectionState : uint8_t
{
	NotConnected,
	Connecting,
	Connected,
	Disconnected
};

struct FMessagePeekResult final
{
	uint16 MessageType;
	uint16 BodySize;
	const char* Body;
};

FString InterpretWsaErrorCodeIfWellKnown(int32 ErrorCode);

class alignas(64) FNetworkWorker final : public FRunnable
{
public:
	static constexpr size_t BufferCapacity = 65536;

	explicit FNetworkWorker(const FString& ServerAddress, const uint16 ServerPort);

	virtual ~FNetworkWorker() override;

	const FString& GetDisconnectedReason();
	
	void Disconnect(FString Reason);
	
	FORCEINLINE ERatkiniaConnectionState GetConnectionState()
	{
		return ConnectionState.load(std::memory_order_relaxed);
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
			Disconnect(FString::Printf(TEXT("메시지 크기가 허용 범위를 초과했습니다: %llu/%llu."), MessageTotalSize, MessageMaxSize));
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
			Disconnect(FString::Printf(TEXT("송신 버퍼에 여유 공간이 없습니다: SendBuffer: %llu/%llu, MessageTotalSize: %llu."), SendBufferAvailableSize, BufferCapacity-1, MessageTotalSize));
			return;
		}

		const MessageHeader Header
		{
			Htons(static_cast<uint16_t>(MessageType)),
			Htons(MessageBodySize)
		};

		const size_t PrimaryHeaderSize = std::min<
			size_t>(MessageHeaderSize, BufferCapacity - LoadedSendBufferTail);
		const size_t SecondaryHeaderSize = MessageHeaderSize - PrimaryHeaderSize;

		memcpy(SendBuffer.Get() + LoadedSendBufferTail, &Header, PrimaryHeaderSize);
		memcpy(SendBuffer.Get(), reinterpret_cast<const char*>(&Header) + PrimaryHeaderSize, SecondaryHeaderSize);

		const size_t TailAfterHeader = (LoadedSendBufferTail + MessageHeaderSize) % BufferCapacity;
		const size_t PrimaryBodySize = std::min<size_t>(MessageBodySize, BufferCapacity - TailAfterHeader);
		const size_t SecondaryBodySize = MessageBodySize - PrimaryBodySize;

		if (PrimaryBodySize == MessageBodySize)
		{
			Message.SerializeToArray(SendBuffer.Get() + TailAfterHeader, MessageBodySize);
		}
		else
		{
			Message.SerializeToArray(SendContiguousPushBuffer.Get(), MessageTotalSize);
			memcpy(SendBuffer.Get() + TailAfterHeader, SendContiguousPushBuffer.Get(), PrimaryBodySize);
			memcpy(SendBuffer.Get(), SendContiguousPushBuffer.Get() + PrimaryBodySize, SecondaryBodySize);
		}

		SendBufferTail.store((LoadedSendBufferTail + MessageTotalSize) % BufferCapacity, std::memory_order_release);
	}

	TOptional<FMessagePeekResult> TryPeekMessage();

	void Pop(const FMessagePeekResult& Result)
	{
		ReceiveBufferHead.store(
			(ReceiveBufferHead.load(std::memory_order_acquire) + RatkiniaProtocol::MessageHeaderSize + Result.BodySize) % BufferCapacity,
			std::memory_order_release);
	}

private:
	OpenSSL::SSL_CTX* const SslCtx;
	OpenSSL::SSL* const Ssl;
	const SOCKET Socket;
	TStaticArray<char, 16> ServerAddrIn;
	
	FRunnableThread* IoThread;
	std::atomic<ERatkiniaConnectionState> ConnectionState;

	FCriticalSection DisconnectedReasonMutex;
	FString DisconnectedReason;
	
	alignas(64) const TUniquePtr<char[]> SendBuffer;
	alignas(64) const TUniquePtr<char[]> SendContiguousPushBuffer;
	alignas(64) std::atomic<size_t> SendBufferHead;
	alignas(64) std::atomic<size_t> SendBufferTail; // Exclusive
	
	alignas(64) const TUniquePtr<char[]> ReceiveBuffer;
	alignas(64) const TUniquePtr<char[]> ReceiveContiguousPopBuffer;
	alignas(64) std::atomic<size_t> ReceiveBufferHead;
	alignas(64) std::atomic<size_t> ReceiveBufferTail; // Exclusive

	explicit FNetworkWorker(const FRatkiniaClientInitResult& InitResult);

public:
	virtual uint32 Run() override;
};
