// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LoginGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ALoginGameMode final : public AGameModeBase, public RatkiniaProtocol::StcStub<ALoginGameMode>
{
	GENERATED_BODY()
	
public:
	explicit ALoginGameMode();
	
	virtual ~ALoginGameMode() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnParseMessageFailed(uint64_t context, RatkiniaProtocol::StcMessageType messagetType) override;

	virtual void OnUnknownMessageType(uint64_t context, RatkiniaProtocol::StcMessageType messagetType) override;

	virtual void OnUnhandledMessageType(uint64_t context, RatkiniaProtocol::StcMessageType messagetType) override;

	virtual void OnLoginResponse(uint64_t context, const bool successful, const std::string& failure_reason) override;
	
	virtual void OnRegisterResponse(uint64_t context, const RatkiniaProtocol::RegisterResponse_FailedReason failed_reason) override;

private:
	TUniquePtr<char[]> MessageBodyBuffer;
};
