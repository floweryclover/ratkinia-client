// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "HumanLikeBodyComponent.h"
#include "DataTableSubsystem.h"
#include "HumanLikeBodyRow.h"
#include "Components/ComponentMessage.gen.pb.h"

void UHumanLikeBodyComponent::UpdateEcsComponent(const RatkiniaProtocol::ComponentVariant& ComponentVariant)
{
	static const FString ContextString = TEXT("UHumanLikeBodyComponent::UpdateEcsComponent");
	const FHumanLikeBodyRow* const Row = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>()->HumanBodyBody->FindRow<FHumanLikeBodyRow>(UTF8_TO_TCHAR(ComponentVariant.human_like_body().static_mesh_name().c_str()), ContextString);
	SetSkeletalMesh(Row->SkeletalMesh.Get());
}
