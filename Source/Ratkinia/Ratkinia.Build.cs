// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.IO;

public class Ratkinia : ModuleRules
{
	public Ratkinia(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OpenSSL" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		string ProjectRoot = Path.Combine(ModuleDirectory, "..", "..");

		string RatkiniaProtocolRoot = Path.Combine(ProjectRoot, "..", "ratkinia-protocol");
		string RatkiniaProtocolClientPath = Path.Combine(RatkiniaProtocolRoot, "Out", "Client");
		string RatkiniaProtocolCommonPath = Path.Combine(RatkiniaProtocolRoot, "Out", "Common");
		PublicIncludePaths.Add(RatkiniaProtocolClientPath);
		PublicIncludePaths.Add(RatkiniaProtocolCommonPath);
		
		string PublicRatkiniaProtocolPath = Path.Combine(ModuleDirectory, "Public", "RatkiniaProtocol");
		PublicIncludePaths.Add(PublicRatkiniaProtocolPath);
		
		string ThirdPartyPath = Path.Combine(ProjectRoot, "ThirdParty");
		string ProtobufPath = Path.Combine(ThirdPartyPath, "Protobuf");
		string ProtobufIncludePath = Path.Combine(ProtobufPath, "include");
		string ProtobufLibPath = Path.Combine(ProtobufPath, "lib");		
		PublicIncludePaths.Add(ProtobufIncludePath);

		foreach (var libFile in Directory.GetFiles(ProtobufLibPath, "*.lib"))
		{
			PublicAdditionalLibraries.Add(libFile);
		}

		PublicDefinitions.Add("PROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0");
		PublicDefinitions.Add("PROTOBUF_BUILTIN_ATOMIC=0");
		CppStandard = CppStandardVersion.Cpp20;
	}
}
