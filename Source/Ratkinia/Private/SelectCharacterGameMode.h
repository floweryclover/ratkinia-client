// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SelectCharacterGameMode.generated.h"

class USelectCharacterWidget;
/**
 * 
 */
UCLASS()
class ASelectCharacterGameMode final : public AGameModeBase, public RatkiniaProtocol::TStcStub<ASelectCharacterGameMode>
{
	GENERATED_BODY()

public:
	explicit ASelectCharacterGameMode();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnUnknownMessageType(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnParseMessageFailed(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnUnhandledMessageType(RatkiniaProtocol::StcMessageType MessageType) override;

	virtual void OnSendMyCharacters(TArrayView<const RatkiniaProtocol::SendMyCharacters_CharacterLoadData* const> CharacterLoadDatas) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USelectCharacterWidget> SelectCharacterWidgetClass;

	UPROPERTY()
	TObjectPtr<USelectCharacterWidget> SelectCharacterWidget;
	
	UFUNCTION()
	void OpenCreateCharacterLevel();
};
