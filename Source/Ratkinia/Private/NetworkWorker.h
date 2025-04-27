#pragma once

#include "RatkiniaProtocol.gen.h"
#include "ScopedNetworkMessage.h"
#include "Ratkinia/Ratkinia.h"
#include <WinSock2.h>
#include <algorithm>
#include <mutex>
#include <sstream>

class FNetworkWorker final
{
public:
	explicit FNetworkWorker(uint64 BufferCapacity);

	~FNetworkWorker();

	void Connect(const FString& ServerAddress, const uint16 ServerPort);

	FORCEINLINE bool IsStopped() const
	{
		return isStopped_.load(std::memory_order_acquire);
	}

	FORCEINLINE bool IsConnected() const
	{
		return isConnected_.load(std::memory_order_acquire);
	}

	FORCEINLINE void ReleaseScopedNetworkMessage(const TScopedNetworkMessage<FNetworkWorker>& Message)
	{
		receiveBufferBegin_.store(
			(receiveBufferBegin_.load(std::memory_order_acquire) + RatkiniaProtocol::MessageHeaderSize + Message.
				GetBodySize()) % BufferCapacity,
			std::memory_order_release);
	}

	FString GetEndReason() const
	{
		return {UTF8_TO_TCHAR(endReason_.c_str())};
	}

	template <typename TMessage>
	void Send(TMessage&& Message, const RatkiniaProtocol::CtsMessageType MessageType)
	{
		using namespace RatkiniaProtocol;

		check(IsInGameThread());

		const auto messageBodySize = Message.ByteSizeLong();
		const auto messageTotalSize = messageBodySize + MessageHeaderSize;

		if (messageTotalSize > MessageMaxSize)
		{
			StoreStopped("메시지 크기가 허용 범위를 초과했습니다:", messageTotalSize);
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
			StoreStopped("송신 버퍼에 여유 공간이 없습니다.");
			return;
		}

		const MessageHeader header{htons(static_cast<uint16_t>(MessageType)), htons(messageBodySize)};

		const auto primaryHeaderSize = std::min<
			size_t>(MessageHeaderSize, BufferCapacity - loadedSendBufferTail);
		const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

		memcpy_s(sendBuffer_.get() + loadedSendBufferTail, primaryHeaderSize, &header, primaryHeaderSize);
		memcpy_s(sendBuffer_.get(), secondaryHeaderSize, reinterpret_cast<const char*>(&header) + primaryHeaderSize,
		         secondaryHeaderSize);

		const auto loadedSendBufferTailAfterHeader = (loadedSendBufferTail + MessageHeaderSize) % BufferCapacity;

		const auto primaryBodySize = std::min<size_t>(messageBodySize, BufferCapacity - loadedSendBufferTailAfterHeader);
		const auto secondaryBodySize = messageBodySize - primaryBodySize;

		if (primaryBodySize == messageBodySize)
		{
			Message.SerializeToArray(sendBuffer_.get() + loadedSendBufferTailAfterHeader, primaryBodySize);
		}
		else
		{
			Message.SerializeToArray(sendTempBuffer_.get(), messageTotalSize);
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

	std::atomic_bool isStopped_;
	std::atomic_bool isConnected_;

	std::mutex endReasonMutex_;
	std::string endReason_;

	const size_t BufferCapacity;
	const std::unique_ptr<char[]> sendTempBuffer_;
	const std::unique_ptr<char[]> sendBuffer_;
	std::atomic<size_t> sendBufferBegin_;
	std::atomic<size_t> sendBufferEnd_; // Exclusive
	const std::unique_ptr<char[]> receiveBuffer_;
	const std::unique_ptr<char[]> receiveTempBuffer_;
	std::atomic<size_t> receiveBufferBegin_;
	std::atomic<size_t> receiveBufferEnd_; // Exclusive

	SOCKET clientSocket_;

	template <typename... Args>
	void StoreStopped(Args&&... args)
	{
		std::lock_guard lockGuard{endReasonMutex_};
		std::ostringstream oss;

		((oss << " " << args), ... );
		endReason_ = oss.str();
		isStopped_.store(true, std::memory_order_release);
	}

	void SendThreadBody();

	void ReceiveThreadBody();
};
