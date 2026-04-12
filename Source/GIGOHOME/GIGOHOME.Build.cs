// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GIGOHOME : ModuleRules
{
	public GIGOHOME(ReadOnlyTargetRules Target) : base(Target)
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
			"NavigationSystem",
			"Niagara",
			"LevelSequence",
			"MovieScene"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GIGOHOME",
			// Variant systems
			"GIGOHOME/Variant_Horror",
			"GIGOHOME/Variant_Horror/UI",
			"GIGOHOME/Variant_Shooter",
			"GIGOHOME/Variant_Shooter/AI",
			"GIGOHOME/Variant_Shooter/UI",
			"GIGOHOME/Variant_Shooter/Weapons",
			// GI Go Home core game systems
			"GIGOHOME/GIGoHome",
			"GIGOHOME/GIGoHome/Player",
			"GIGOHOME/GIGoHome/Player/Components",
			"GIGOHOME/GIGoHome/Squad",
			"GIGOHOME/GIGoHome/AI",
			"GIGOHOME/GIGoHome/Tunnels",
			"GIGOHOME/GIGoHome/Traps",
			"GIGOHOME/GIGoHome/Foliage",
			"GIGOHOME/GIGoHome/Audio",
			"GIGOHOME/GIGoHome/Wave",
			"GIGOHOME/GIGoHome/Weather",
			"GIGOHOME/GIGoHome/Weapons",
			// Campaign system
			"GIGOHOME/GIGoHome/Campaign",
			"GIGOHOME/GIGoHome/Campaign/Actors",
			"GIGOHOME/GIGoHome/Campaign/Components"
		});
	}
}
