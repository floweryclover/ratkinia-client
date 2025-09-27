// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "RatkiniaComponentSubsystem.h"
#include "HumanLikeBodyComponent.h"

void URatkiniaComponentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Register<UHumanLikeBodyComponent>();
}

TArray<TUniquePtr<FRawSparseSet>> URatkiniaComponentSubsystem::CreateSparseSets()
{
	TArray<TUniquePtr<FRawSparseSet>> SparseSets;
	Algo::Transform(CreateSparseSetFunctions,
		SparseSets,
		[this](TUniquePtr<FRawSparseSet> (*CreateSparseSet)())
	{
		return CreateSparseSet();
	});

	return SparseSets;
}