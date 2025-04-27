#pragma once

#include "CTS_Auth.pb.h"
#include "RatkiniaProtocol.h"
#include <WinSock2.h>
#include <algorithm>
#include <mutex>
#include <sstream>

#include "CoreMinimal.h"

class FNetworkWorker final : public FRunnable
{
public:
	explicit FNetworkWorker(uint64 BufferCapacity, const FString& ServerAddress, const uint16 ServerPort);

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	FORCEINLINE bool IsStopped() const
	{
		return isStopped_.load(std::memory_order_acquire);
	}

	FORCEINLINE bool IsConnected() const
	{
		return isConnected_.load(std::memory_order_acquire);
	}

	FString GetEndReason() const
	{
		return {UTF8_TO_TCHAR(endReason_.c_str())};
	}

	template <typename TMessage>
	void Send(TMessage Message, const RatkiniaProtocol::CtsMessageType MessageType)
	{
		using namespace RatkiniaProtocol;

		check(IsInGameThread());

		const auto messageBodySize = Message.ByteSizeLong();
		const auto messageTotalSize = messageBodySize + MessageHeaderSize;

		if (messageTotalSize > MessageMaxSize)
		{
			std::lock_guard lockGuard{endReasonMutex_};
			endReason_ = "메시지 크기가 허용 범위를 초과했습니다: ";
			endReason_.append(std::to_string(messageTotalSize));
			isStopped_.store(true, std::memory_order_release);
			return;
		}

		const auto availableSize = BufferCapacity - sendBufferSize_.load(std::memory_order_acquire);
		if (availableSize < messageTotalSize)
		{
			std::lock_guard lockGuard{endReasonMutex_};
			endReason_ = "송신 버퍼에 여유 공간이 없습니다.";
			isStopped_.store(true, std::memory_order_release);
			return;
		}

		const MessageHeader header{htons(static_cast<uint16_t>(MessageType)), htons(messageBodySize)};

		const auto sendBufferEnd_beforeHeader = sendBufferEnd_.load(std::memory_order_acquire);
		const auto primaryHeaderSize = std::min<
			size_t>(MessageHeaderSize, BufferCapacity - sendBufferEnd_beforeHeader);
		const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

		memcpy_s(sendBuffer_.get() + sendBufferEnd_beforeHeader, primaryHeaderSize, &header, primaryHeaderSize);
		memcpy_s(sendBuffer_.get(), secondaryHeaderSize, reinterpret_cast<const char*>(&header) + primaryHeaderSize,
		         secondaryHeaderSize);

		const auto sendBufferEnd_afterHeader = (sendBufferEnd_beforeHeader + MessageHeaderSize) % BufferCapacity;

		const auto primaryBodySize = std::min<size_t>(messageBodySize, BufferCapacity - sendBufferEnd_afterHeader);
		const auto secondaryBodySize = messageBodySize - primaryBodySize;

		if (primaryBodySize == messageBodySize)
		{
			Message.SerializeToArray(sendBuffer_.get() + sendBufferEnd_afterHeader, primaryBodySize);
		}
		else
		{
			Message.SerializeToArray(sendTempBuffer_.get(), messageTotalSize);
			memcpy_s(sendBuffer_.get() + sendBufferEnd_afterHeader, primaryBodySize, sendTempBuffer_.get(),
			         primaryBodySize);
			memcpy_s(sendBuffer_.get(), secondaryBodySize, sendTempBuffer_.get() + primaryBodySize, secondaryBodySize);
		}

		sendBufferEnd_.store((sendBufferEnd_afterHeader + messageBodySize) % BufferCapacity,
		                     std::memory_order_release);
		sendBufferSize_.fetch_add(messageTotalSize, std::memory_order_release);
	}

	bool TryPopMessage(uint16& OutMessageType, uint16& OutBodyLength, char* const OutMessageBodyBuffer, const int32 InMessageBodyBufferSize)
	{
		using namespace RatkiniaProtocol;

		const auto receiveBufferSize = receiveBufferSize_.load(std::memory_order_acquire);
		if (receiveBufferSize < MessageHeaderSize)
		{
			return false;
		}
		
		MessageHeader header{};
		const auto receiveBufferBegin_beforeHeader = receiveBufferBegin_.load(std::memory_order_acquire);
		const auto primaryHeaderSize = std::min<size_t>(MessageHeaderSize, BufferCapacity - receiveBufferBegin_beforeHeader);
		const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

		memcpy_s(&header, primaryHeaderSize, receiveBuffer_.get() + receiveBufferBegin_beforeHeader, primaryHeaderSize);
		memcpy_s(reinterpret_cast<char*>(&header) + primaryHeaderSize, secondaryHeaderSize, receiveBuffer_.get(), secondaryHeaderSize);
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
			memcpy_s(OutMessageBodyBuffer, header.BodyLength, receiveBuffer_.get() + receiveBufferBegin_afterHeader, header.BodyLength);
		}
		else
		{
			memcpy_s(OutMessageBodyBuffer, primaryBodySize, receiveBuffer_.get() + receiveBufferBegin_afterHeader, primaryBodySize);
			memcpy_s(OutMessageBodyBuffer + primaryBodySize, secondaryBodySize, receiveBuffer_.get(), secondaryBodySize);
		}

		receiveBufferBegin_.store((receiveBufferBegin_afterHeader + header.BodyLength) % BufferCapacity, std::memory_order_release);
		receiveBufferSize_.fetch_sub(messageTotalSize, std::memory_order_release);
		return true;
	}

private:
	std::atomic_bool isStopped_;
	std::atomic_bool isConnected_;

	std::mutex endReasonMutex_;
	std::string endReason_;

	const size_t BufferCapacity;
	const std::unique_ptr<char[]> sendTempBuffer_;
	const std::unique_ptr<char[]> sendBuffer_;
	std::atomic<size_t> sendBufferBegin_;
	std::atomic<size_t> sendBufferEnd_; // Exclusive
	std::atomic<size_t> sendBufferSize_;
	const std::unique_ptr<char[]> receiveBuffer_;
	std::atomic<size_t> receiveBufferBegin_;
	std::atomic<size_t> receivedBufferEnd_; // Exclusive
	std::atomic<size_t> receiveBufferSize_;

	SOCKET clientSocket_;
	std::string serverAddress_;
	uint16_t serverPort_;

	template <typename... Args>
	void StoreStopped(Args&&... args)
	{
		std::lock_guard lockGuard{endReasonMutex_};
		std::ostringstream oss;

		((oss << " " << args), ... );
		endReason_ = oss.str();
		isStopped_.store(true, std::memory_order_release);
	}
};
