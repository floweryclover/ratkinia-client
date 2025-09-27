// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "WorldGameMode.h"

#include "EntityComponent.h"
#include "PossessableEntity.h"
#include "RatkiniaClientSubsystem.h"
#include "RatkiniaComponentSubsystem.h"
#include "Ratkinia/Ratkinia.h"

#include "Kismet/GameplayStatics.h"

using namespace RatkiniaProtocol;

AWorldGameMode::AWorldGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWorldGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URatkiniaClientSubsystem* const Client = GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>();
	if (Client->GetConnectionState() != ERatkiniaConnectionState::Connected)
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/Login/Login"), true);
		return;
	}

	while (const auto Message = Client->TryPeekMessage())
	{
		HandleStc(Message->MessageType, Message->BodySize, Message->Body);
		Client->PopMessage(*Message);
	}
}

void AWorldGameMode::OnSpawnEntity(const TArrayView<const SpawnEntity_Data* const> EntitySpawnDatas)
{
	for (const SpawnEntity_Data* const Data : EntitySpawnDatas)
	{
		check(!EntityComponents.IsValidIndex(Data->entity_id()));
		
		AActor* const Entity = [&]() -> AActor*
		{
			if (Data->type() == SpawnEntity_Type_Normal)
			{
				AActor* const NormalEntity = GetWorld()->SpawnActor<AActor>();
				return NormalEntity;
			}

			APossessableEntity* const MyCharacter = GetWorld()->SpawnActor<APossessableEntity>();
			GetWorld()->GetFirstPlayerController()->Possess(MyCharacter);
			return MyCharacter;
		}();
		check(IsValid(Entity));

		UEntityComponent* const EntityComponent = Cast<UEntityComponent>(Entity->AddComponentByClass(UEntityComponent::StaticClass(), false, {}, false));
		check(IsValid(EntityComponent));
		EntityComponent->Init(Data->entity_id());
		EntityComponents.EmplaceAt(Data->entity_id(), EntityComponent);
	}
}

void AWorldGameMode::OnUnknownMessageType(const StcMessageType MessageType)
{
	UE_LOG(LogRatkinia, Fatal, TEXT("알 수 없는 메시지 타입: %d"), MessageType)
	checkNoEntry();
}

void AWorldGameMode::OnParseMessageFailed(const StcMessageType MessageType)
{
	UE_LOG(LogRatkinia, Fatal, TEXT("메시지 역직렬화 실패: %d"), MessageType)
	checkNoEntry();
}

void AWorldGameMode::OnUnhandledMessageType(const StcMessageType MessageType)
{
	UE_LOG(LogRatkinia, Fatal, TEXT("처리되지 않은 메시지 타입: %d"), MessageType)
	checkNoEntry();
}

void AWorldGameMode::OnAttachComponent(const TArrayView<const AttachComponent_Data* const> ComponentAttachDatas)
{
	const URatkiniaComponentSubsystem* const ComponentSubsystem = GetGameInstance()->GetSubsystem<
		URatkiniaComponentSubsystem>();
	check(IsValid(ComponentSubsystem));

	for (const AttachComponent_Data* const Data : ComponentAttachDatas)
	{
		if (!EntityComponents.IsValidIndex(Data->target_entity()) || !IsValid(EntityComponents[Data->target_entity()]))
		{
			GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->ClearSession(TEXT("엔티티 무결성이 손상되었습니다."));
			return;
		}
		UEntityComponent& EntityComponent = *EntityComponents[Data->target_entity()];
		const uint16 RuntimeOrder = Data->component_runtime_order();
		checkf(RuntimeOrder > 0 || RuntimeOrder < SparseSets.Num() || !SparseSets[RuntimeOrder],
		       TEXT("준비되지 않은 Sparse Set: %d"), RuntimeOrder);
		SparseSets[RuntimeOrder]->AttachComponentTo(EntityComponent);
	}
}

void AWorldGameMode::BeginPlay()
{
	Super::BeginPlay();

	URatkiniaComponentSubsystem* const ComponentSubsystem = GetGameInstance()->GetSubsystem<
		URatkiniaComponentSubsystem>();
	check(IsValid(ComponentSubsystem));

	SparseSets = ComponentSubsystem->CreateSparseSets();
}
