// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"
#include "SparseSet.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WorldGameMode.generated.h"

class UMessageBoxWidget;
class UEntityComponent;
class APossessableEntity;

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
	virtual void OnAttachComponent(TArrayView<const RatkiniaProtocol::AttachComponent_Data* const> ComponentAttachDatas) override;
	virtual void OnUpdateComponent(TArrayView<const RatkiniaProtocol::UpdateComponent_Data* const> ComponentUpdateDatas) override;
	virtual void OnNotify(RatkiniaProtocol::Notify_Type Type, FString Text) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APossessableEntity> PossessableEntityClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMessageBoxWidget> MessageBoxWidgetClass;

	TSparseArray<TObjectPtr<UEntityComponent>> EntityComponents;
	TArray<TUniquePtr<FRawSparseSet>> SparseSets;

	void PopupMessageBoxWidget(FText Text);
};
