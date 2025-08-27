// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"
#include <google/protobuf/arena.h>

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WorldGameMode.generated.h"

class APossessableEntity;
class IEntity;
/**
 * 
 */
UCLASS()
class AWorldGameMode final : public AGameModeBase, public RatkiniaProtocol::TStcStub<AWorldGameMode>
{
	GENERATED_BODY()

public:
	explicit AWorldGameMode();
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnUnknownMessageType(RatkiniaProtocol::StcMessageType MessageType) override;
	virtual void OnParseMessageFailed(RatkiniaProtocol::StcMessageType MessageType) override;
	virtual void OnUnhandledMessageType(RatkiniaProtocol::StcMessageType MessageType) override;
	virtual void OnSpawnEntity(TArrayView<const RatkiniaProtocol::SpawnEntity_Data* const> EntitySpawnDatas) override;
	virtual void OnAttachComponentTo(
		TArrayView<const RatkiniaProtocol::AttachComponentTo_Data* const> ComponentAttachDatas) override;

	google::protobuf::Arena* GetArena()
	{
		return &Arena;
	}
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APossessableEntity> PossessableEntityClass;
	
	virtual void BeginPlay() override;

private:
	google::protobuf::Arena Arena;
	TSparseArray<IEntity*> Entities;
};
