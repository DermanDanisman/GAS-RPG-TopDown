// © 2025 Heathrow (Derman). All rights reserved.

using UnrealBuildTool;

public class GASCore : ModuleRules
{
	public GASCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject", 
				"Engine",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"NetCore",
				"UMG", // For UI widgets
				"EnhancedInput" // For modern input system
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"ToolMenus",
				"EditorStyle",
				"EditorWidgets",
				"UnrealEd",
				"PropertyEditor",
				"KismetCompiler",
				"BlueprintGraph",
				"AIModule" // For AI integration
			}
		);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorStyle",
					"EditorWidgets",
					"UnrealEd",
					"ToolMenus"
				}
			);
		}
	}
}