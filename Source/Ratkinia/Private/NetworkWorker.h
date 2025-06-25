#pragma once

#include "RatkiniaProtocol.gen.h"
#include "ScopedNetworkMessage.h"
#include <WinSock2.h>
#include <mutex>
#include <format>

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

	explicit FNetworkWorker();

	~FNetworkWorker();

	void Connect(const FString& serverAddress, const uint16 serverPort);

	const std::string& GetDisconnectedReason();
	
	void Disconnect(std::string reason);
	
	FORCEINLINE ERatkiniaConnectionState GetConnectionState()
	{
		return connectionState_.load(std::memory_order_acquire);
	}

	FORCEINLINE void ReleaseScopedNetworkMessage(const TScopedNetworkMessage<FNetworkWorker>& message)
	{
		receiveBufferBegin_.store(
			(receiveBufferBegin_.load(std::memory_order_acquire) + RatkiniaProtocol::MessageHeaderSize + message.
				GetBodySize()) % BufferCapacity,
			std::memory_order_release);
	}
	
	template <typename TMessage>
	void Send(TMessage&& message, const RatkiniaProtocol::CtsMessageType messageType)
	{
		using namespace RatkiniaProtocol;

		check(IsInGameThread());

		const auto messageBodySize = message.ByteSizeLong();
		const auto messageTotalSize = messageBodySize + MessageHeaderSize;

		if (messageTotalSize > MessageMaxSize)
		{
			Disconnect(std::format("메시지 크기가 허용 범위를 초과했습니다: {}.", messageTotalSize));
			return;
		}

		const auto loadedSendBufferHead{sendBufferBegin_.load(std::memory_order_acquire)};
		const auto loadedSendBufferTail{sendBufferEnd_.load(std::memory_order_acquire)};
		const auto sendBufferSize{
			loadedSendBufferTail >= loadedSendBufferHead
				? loadedSendBufferTail - loadedSendBufferHead
				: loadedSendBufferTail + BufferCapacity - loadedSendBufferHead
		};
		const auto availableSize{BufferCapacity - sendBufferSize - 1};
		if (availableSize < messageTotalSize)
		{
			Disconnect("송신 버퍼에 여유 공간이 없습니다.");
			return;
		}

		const MessageHeader header{htons(static_cast<uint16_t>(messageType)), htons(messageBodySize)};

		const auto primaryHeaderSize = std::min<
			size_t>(MessageHeaderSize, BufferCapacity - loadedSendBufferTail);
		const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

		memcpy_s(sendBuffer_.get() + loadedSendBufferTail, primaryHeaderSize, &header, primaryHeaderSize);
		memcpy_s(sendBuffer_.get(), secondaryHeaderSize, reinterpret_cast<const char*>(&header) + primaryHeaderSize,
		         secondaryHeaderSize);

		const auto loadedSendBufferTailAfterHeader = (loadedSendBufferTail + MessageHeaderSize) % BufferCapacity;

		const auto primaryBodySize = std::min<
			size_t>(messageBodySize, BufferCapacity - loadedSendBufferTailAfterHeader);
		const auto secondaryBodySize = messageBodySize - primaryBodySize;

		if (primaryBodySize == messageBodySize)
		{
			message.SerializeToArray(sendBuffer_.get() + loadedSendBufferTailAfterHeader, primaryBodySize);
		}
		else
		{
			message.SerializeToArray(sendTempBuffer_.get(), messageTotalSize);
			memcpy_s(sendBuffer_.get() + loadedSendBufferTailAfterHeader, primaryBodySize, sendTempBuffer_.get(),
			         primaryBodySize);
			memcpy_s(sendBuffer_.get(), secondaryBodySize, sendTempBuffer_.get() + primaryBodySize, secondaryBodySize);
		}

		sendBufferEnd_.store((loadedSendBufferTailAfterHeader + messageBodySize) % BufferCapacity,
		                     std::memory_order_release);
	}

	TOptional<TScopedNetworkMessage<FNetworkWorker>> TryPopMessage();

private:
	std::thread sendThread_;
	std::thread receiveThread_;

	std::mutex disconnectedReasonMutex_;
	std::string disconnectedReason_;

	alignas(64) SOCKET clientSocket_;
	alignas(64) std::atomic<ERatkiniaConnectionState> connectionState_;
	alignas(64) const std::unique_ptr<char[]> sendTempBuffer_;
	alignas(64) const std::unique_ptr<char[]> sendBuffer_;
	alignas(64) std::atomic<size_t> sendBufferBegin_;
	alignas(64) std::atomic<size_t> sendBufferEnd_; // Exclusive
	alignas(64) const std::unique_ptr<char[]> receiveBuffer_;
	alignas(64) const std::unique_ptr<char[]> receiveTempBuffer_;
	alignas(64) std::atomic<size_t> receiveBufferBegin_;
	alignas(64) std::atomic<size_t> receiveBufferEnd_; // Exclusive
	
	void SendThreadBody();

	void ReceiveThreadBody();

	void Cleanup();
};
