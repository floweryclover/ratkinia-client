// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "WorldGameMode.h"

#include "Entity.h"
#include "NormalEntity.h"
#include "PossessableEntity.h"
#include "RatkiniaClientSubsystem.h"
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

void AWorldGameMode::OnSpawnEntity(const TArrayView<const SpawnEntity_Data* const> EntitySpawnDatas)
{
	for (const SpawnEntity_Data* const Data : EntitySpawnDatas)
	{
		IEntity* const Entity = [&]() -> IEntity*
		{
			if (Data->type() == SpawnEntity_Type_Normal)
			{
				return GetWorld()->SpawnActor<ANormalEntity>();
			}
			APossessableEntity* const MyCharacter = GetWorld()->SpawnActor<APossessableEntity>(PossessableEntityClass, FVector{0.0f, 0.0f, 3000.0f}, FRotator::ZeroRotator);
			GetWorld()->GetFirstPlayerController()->Possess(MyCharacter);
			return MyCharacter;
		}();
		Entity->SetEntityId(Data->entity_id());
		Entities.EmplaceAt(Data->entity_id(), Entity);
	}
}

void AWorldGameMode::OnAttachComponentTo(
	const TArrayView<const AttachComponentTo_Data* const> ComponentAttachDatas)
{
	for (const AttachComponentTo_Data* const Data : ComponentAttachDatas)
	{
		switch (Data->component_variant().value_case())
		{
		case ComponentVariant::kNameTag:
			{
				UE_LOG(LogRatkinia, Log, TEXT("이름: %s"), UTF8_TO_TCHAR(Data->component_variant().name_tag().name().c_str()))
				break;
			}
		}
	}
}

void AWorldGameMode::BeginPlay()
{
	Super::BeginPlay();
}
