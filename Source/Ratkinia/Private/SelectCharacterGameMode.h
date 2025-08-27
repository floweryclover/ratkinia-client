// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"
#include <google/protobuf/arena.h>

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

	virtual void OnSendMyCharacters(TArrayView<const RatkiniaProtocol::SendMyCharacters_Data* const> CharacterLoadDatas) override;

	virtual void OnOpenWorld() override;

	virtual void OnNotificate(RatkiniaProtocol::Notificate_Type Type, FString Text) override;

	google::protobuf::Arena* GetArena()
	{
		return &Arena;
	}

protected:
	virtual void BeginPlay() override;
	
private:
	bool bBreakMessagePopLoop;

	google::protobuf::Arena Arena;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USelectCharacterWidget> SelectCharacterWidgetClass;

	UPROPERTY()
	TObjectPtr<USelectCharacterWidget> SelectCharacterWidget;
	
	UFUNCTION()
	void OpenCreateCharacterLevel();

	UFUNCTION()
	void SelectCharacter(int32 Id);
};
