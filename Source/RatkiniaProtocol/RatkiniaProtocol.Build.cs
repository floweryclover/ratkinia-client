// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class RatkiniaProtocol : ModuleRules
{
	public RatkiniaProtocol(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {  });

		PrivateDependencyModuleNames.AddRange(new string[] { "Core" });
		
		string ProjectRoot = Path.Combine(ModuleDirectory, "..", "..");
		
		string ThirdPartyPath = Path.Combine(ProjectRoot, "ThirdParty");
		string ProtobufPath = Path.Combine(ThirdPartyPath, "Protobuf");
		string ProtobufIncludePath = Path.Combine(ProtobufPath, "include");
		string ProtobufLibPath = Path.Combine(ProtobufPath, "lib");
		string RatkiniaProtocolRoot = Path.Combine(ProjectRoot, "..", "ratkinia-protocol");
		string RatkiniaProtocolClientPath = Path.Combine(RatkiniaProtocolRoot, "Out", "Client", "Include");
		PublicIncludePaths.Add(RatkiniaProtocolClientPath);
		PublicIncludePaths.Add(ProtobufIncludePath);

		foreach (var libFile in Directory.GetFiles(ProtobufLibPath, "*.lib"))
		{
			PublicAdditionalLibraries.Add(libFile);
		}
		PublicDefinitions.Add("PROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0");
		PublicDefinitions.Add("PROTOBUF_BUILTIN_ATOMIC=0");
	}
}
