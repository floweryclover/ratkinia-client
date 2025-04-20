// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.IO;

public class Ratkinia : ModuleRules
{
	public Ratkinia(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		string ProjectRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
		string ThirdPartyPath = Path.Combine(ProjectRoot, "ThirdParty");
		
		string ProtobufPath = Path.Combine(ThirdPartyPath, "Protobuf");
		string ProtobufIncludePath = Path.Combine(ProtobufPath, "include");
		string ProtobufLibPath = Path.Combine(ProtobufPath, "lib");		
		
		string OpenSSLPath = Path.Combine(ThirdPartyPath, "OpenSSL");
		string OpenSSLIncludePath = Path.Combine(OpenSSLPath, "include");
		string OpenSSLLibPath = Path.Combine(OpenSSLPath, "lib");

		PublicIncludePaths.Add(ProtobufIncludePath);
		PublicIncludePaths.Add(OpenSSLIncludePath);

		foreach (var libFile in Directory.GetFiles(ProtobufLibPath, "*.lib"))
		{
			PublicAdditionalLibraries.Add(libFile);
		}
		foreach (var libFile in Directory.GetFiles(OpenSSLLibPath, "*.lib"))
		{
			PublicAdditionalLibraries.Add(libFile);
		}

		PublicDefinitions.Add("PROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0");
		PublicDefinitions.Add("PROTOBUF_BUILTIN_ATOMIC=0");
		CppStandard = CppStandardVersion.Cpp20;
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
