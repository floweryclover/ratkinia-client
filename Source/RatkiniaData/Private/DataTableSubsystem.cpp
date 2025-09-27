// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "DataTableSubsystem.h"

UDataTableSubsystem::UDataTableSubsystem()
	: HumanBodyBody
	{
		[]
		{
			static ConstructorHelpers::FObjectFinder<UDataTable> HumanLikeBodyDataTableFinder = TEXT("/Game/Assets/DataTables/DT_HumanLikeBody.DT_HumanLikeBody");
			check(HumanLikeBodyDataTableFinder.Succeeded());
			return HumanLikeBodyDataTableFinder.Object;
		}()
	}
{
}
