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

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		string ProjectRoot = Path.Combine(ModuleDirectory, "..", "..");

		string RatkiniaProtocolRoot = Path.Combine(ProjectRoot, "..", "ratkinia-protocol");
		string RatkiniaProtocolClientPath = Path.Combine(RatkiniaProtocolRoot, "Out", "Client");
		string RatkiniaProtocolCommonPath = Path.Combine(RatkiniaProtocolRoot, "Out", "Common");
		PrivateIncludePaths.Add(RatkiniaProtocolClientPath);
		PrivateIncludePaths.Add(RatkiniaProtocolCommonPath);
		
		string ThirdPartyPath = Path.Combine(ProjectRoot, "ThirdParty");
		string ProtobufPath = Path.Combine(ThirdPartyPath, "Protobuf");
		string ProtobufIncludePath = Path.Combine(ProtobufPath, "include");
		string ProtobufLibPath = Path.Combine(ProtobufPath, "lib");		
		PrivateIncludePaths.Add(ProtobufIncludePath);

		foreach (var libFile in Directory.GetFiles(ProtobufLibPath, "*.lib"))
		{
			PublicAdditionalLibraries.Add(libFile);
		}

		PrivateDefinitions.Add("PROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0");
		PrivateDefinitions.Add("PROTOBUF_BUILTIN_ATOMIC=0");
		CppStandard = CppStandardVersion.Cpp20;
	}
}
