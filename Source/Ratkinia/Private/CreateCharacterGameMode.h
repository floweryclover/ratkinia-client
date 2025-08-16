// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CreateCharacterGameMode.generated.h"

class UCreateCharacterWidget;
/**
 * 
 */
UCLASS()
class ACreateCharacterGameMode final : public AGameModeBase, public RatkiniaProtocol::StcStub<ACreateCharacterGameMode>
{
	GENERATED_BODY()

public:
	explicit ACreateCharacterGameMode();
	
	virtual void OnUnknownMessageType(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnParseMessageFailed(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnUnhandledMessageType(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnCreateCharacterResponse(RatkiniaProtocol::CreateCharacterResponse_CreateCharacterResult Successful) override;

protected:
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCreateCharacterWidget> CreateCharacterWidgetClass;

	UFUNCTION()
	void CreateCharacter(FText Name);

	UFUNCTION()
	void OpenSelectCharacterLevel();
};
