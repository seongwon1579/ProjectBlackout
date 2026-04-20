// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectBlackout : ModuleRules
{
	public ProjectBlackout(ReadOnlyTargetRules Target) : base(Target)
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
			"SlateCore",
			// GAS (Gameplay Ability System)
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectBlackout",
			"ProjectBlackout/Core",
			"ProjectBlackout/GameplayTags",
			"ProjectBlackout/Interfaces",
			"ProjectBlackout/Data",
			"ProjectBlackout/GAS",
			"ProjectBlackout/GAS/Attributes",
			"ProjectBlackout/GAS/Abilities",
			"ProjectBlackout/GAS/Effects",
			"ProjectBlackout/Characters",
			"ProjectBlackout/Framework",
			"ProjectBlackout/Pool",
			"ProjectBlackout/Variant_Platforming",
			"ProjectBlackout/Variant_Platforming/Animation",
			"ProjectBlackout/Variant_Combat",
			"ProjectBlackout/Variant_Combat/AI",
			"ProjectBlackout/Variant_Combat/Animation",
			"ProjectBlackout/Variant_Combat/Gameplay",
			"ProjectBlackout/Variant_Combat/Interfaces",
			"ProjectBlackout/Variant_Combat/UI",
			"ProjectBlackout/Variant_SideScrolling",
			"ProjectBlackout/Variant_SideScrolling/AI",
			"ProjectBlackout/Variant_SideScrolling/Gameplay",
			"ProjectBlackout/Variant_SideScrolling/Interfaces",
			"ProjectBlackout/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
