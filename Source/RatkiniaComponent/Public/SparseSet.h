// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "IEcsComponent.h"
#include "EntityComponent.h"
#include <type_traits>

template<typename TComponent>
concept IsComponent = std::is_final_v<TComponent> && std::is_base_of_v<IEcsComponent, TComponent> && std::is_base_of_v<UActorComponent, TComponent>;

class FRawSparseSet
{
public:
	virtual ~FRawSparseSet() = default;
	
	virtual void AttachComponentTo(UEntityComponent& EntityComponent) = 0;
	
	virtual void UpdateComponentOf(UEntityComponent& EntityComponent, const RatkiniaProtocol::ComponentVariant& ComponentVariant) = 0;
};

template<IsComponent TComponent>
class TSparseSet final : public FRawSparseSet
{
public:
	virtual void AttachComponentTo(UEntityComponent& EntityComponent) override
	{
		check(!Components.IsValidIndex(EntityComponent.GetEntityId()));
	
		TComponent* const Component = Cast<TComponent>(EntityComponent.GetOwner()->AddComponentByClass(TComponent::StaticClass(), false, {}, false));
		check(IsValid(Component));

		if constexpr (std::is_base_of_v<USceneComponent, TComponent>)
		{
			USceneComponent* const RootComponent = EntityComponent.GetOwner()->GetRootComponent();
			check(IsValid(RootComponent));
			Component->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	
		Components.EmplaceAt(EntityComponent.GetEntityId(), Component);
	}
	
	virtual void UpdateComponentOf(UEntityComponent& EntityComponent, const RatkiniaProtocol::ComponentVariant& ComponentVariant) override
	{
		check(Components.IsValidIndex(EntityComponent.GetEntityId()));
		check(IsValid(Components[EntityComponent.GetEntityId()]));
	
		static_cast<TComponent*>(Components[EntityComponent.GetEntityId()])->UpdateEcsComponent(ComponentVariant);
	}

private:
	TSparseArray<TObjectPtr<TComponent>> Components;
};

