// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class fastGAS_Demo : ModuleRules
{
	public fastGAS_Demo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"fastGAS_Demo",
			"fastGAS_Demo/Variant_Platforming",
			"fastGAS_Demo/Variant_Platforming/Animation",
			"fastGAS_Demo/Variant_Combat",
			"fastGAS_Demo/Variant_Combat/AI",
			"fastGAS_Demo/Variant_Combat/Animation",
			"fastGAS_Demo/Variant_Combat/Gameplay",
			"fastGAS_Demo/Variant_Combat/Interfaces",
			"fastGAS_Demo/Variant_Combat/UI",
			"fastGAS_Demo/Variant_SideScrolling",
			"fastGAS_Demo/Variant_SideScrolling/AI",
			"fastGAS_Demo/Variant_SideScrolling/Gameplay",
			"fastGAS_Demo/Variant_SideScrolling/Interfaces",
			"fastGAS_Demo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
