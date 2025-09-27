// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "SparseSet.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RatkiniaComponentSubsystem.generated.h"

namespace RatkiniaProtocol
{
	class ComponentVariant;
}

/**
 * 
 */
UCLASS()
class RATKINIACOMPONENT_API URatkiniaComponentSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	TArray<TUniquePtr<FRawSparseSet>> CreateSparseSets();
	
private:
	TArray<TUniquePtr<FRawSparseSet>(*)()> CreateSparseSetFunctions; // 인덱스가 곧 Component Order

	template<typename TComponent>
	void Register()
	{
		const uint16 RuntimeOrder = TComponent::RuntimeOrder;
		if (RuntimeOrder >= CreateSparseSetFunctions.Num())
		{
			CreateSparseSetFunctions.AddZeroed(RuntimeOrder - CreateSparseSetFunctions.Num() + 1);
		}
		checkf(CreateSparseSetFunctions[RuntimeOrder] == nullptr, TEXT("중복 등록된 컴포넌트: %d"), RuntimeOrder);
		CreateSparseSetFunctions[RuntimeOrder] = []() -> TUniquePtr<FRawSparseSet> { return MakeUnique<TSparseSet<TComponent>>(); };
	}
};
