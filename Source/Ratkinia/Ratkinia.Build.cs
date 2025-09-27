// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.IO;

public class Ratkinia : ModuleRules
{
	public Ratkinia(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "OpenSSL" });

		PrivateDependencyModuleNames.AddRange(new string[] { "RatkiniaComponent", "RatkiniaProtocol", "RatkiniaData" });
	}
}
