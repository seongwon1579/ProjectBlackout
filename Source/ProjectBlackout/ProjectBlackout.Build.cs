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
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"AnimGraphRuntime",
			// GAS (Gameplay Ability System)
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"Niagara",
			"PhysicsCore",
			// Networking (Matchmaking System)
			"HTTP",
			"Json",
			"JsonUtilities",
			"WebSockets",
			"DeveloperSettings",
			// Plugin
			"MotionWarping"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {"MoviePlayer" });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectBlackout",
			"ProjectBlackout/Core",
			"ProjectBlackout/Combat",
			"ProjectBlackout/Combat/Weapons",
			"ProjectBlackout/Combat/Components",
			"ProjectBlackout/GameplayTags",
			"ProjectBlackout/Interfaces",
			"ProjectBlackout/Data",
			"ProjectBlackout/GAS",
			"ProjectBlackout/GAS/Attributes",
			"ProjectBlackout/GAS/Abilities",
			"ProjectBlackout/GAS/Effects",
			"ProjectBlackout/GAS/Cues",
			"ProjectBlackout/Characters",
			"ProjectBlackout/AI",
			"ProjectBlackout/Framework",
			"ProjectBlackout/Pool"
		});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
