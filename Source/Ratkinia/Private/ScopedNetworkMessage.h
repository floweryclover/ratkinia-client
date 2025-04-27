// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
template <typename TOwner>
struct TScopedNetworkMessage final
{
	FORCEINLINE explicit TScopedNetworkMessage(TOwner* Owner,
	                                           const uint64 Context,
	                                           const uint16 MessageType,
	                                           const uint16 BodySize,
	                                           const char* const Body)
		: Owner{Owner},
		  Context{Context},
		  MessageType{MessageType},
		  BodySize{BodySize},
		  Body{Body}
	{
	}

	FORCEINLINE ~TScopedNetworkMessage()
	{
		if (Owner)
		{
			Owner->ReleaseScopedNetworkMessage(*this);
		}
	}

	FORCEINLINE TScopedNetworkMessage(TScopedNetworkMessage&& Rhs) noexcept
		: Owner{Rhs.Owner},
		  Context{Rhs.Context},
		  MessageType{Rhs.MessageType},
		  BodySize{Rhs.BodySize},
		  Body{Rhs.Body}
	{
		Rhs.Owner = nullptr;
		Rhs.Context = 0;
		Rhs.MessageType = 0;
		Rhs.BodySize = 0;
		Rhs.Body = nullptr;
	}

	FORCEINLINE TScopedNetworkMessage& operator=(TScopedNetworkMessage&& Rhs) noexcept
	{
		Owner = Rhs.Owner;
		Context = Rhs.Context;
		MessageType = Rhs.MessageType;
		BodySize = Rhs.BodySize;
		Body = Rhs.Body;

		Rhs.Owner = nullptr;
		Rhs.Context = 0;
		Rhs.MessageType = 0;
		Rhs.BodySize = 0;
		Rhs.Body = nullptr;

		return *this;
	}

	TScopedNetworkMessage(const TScopedNetworkMessage&) = delete;

	TScopedNetworkMessage& operator=(const TScopedNetworkMessage&) = delete;

	FORCEINLINE uint64 GetContext() const { return Context; }
	FORCEINLINE uint16 GetMessageType() const { return MessageType; }
	FORCEINLINE uint16 GetBodySize() const { return BodySize; }
	FORCEINLINE const char* GetBody() const { return Body; };

private:
	TOwner* Owner;
	uint64 Context;
	uint16 MessageType;
	uint16 BodySize;
	const char* Body;
};
