using UnrealBuildTool;

public class GASCoreUI : ModuleRules
{
    public GASCoreUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Slate", 
            "SlateCore",
            "Core", 
            "CoreUObject", 
            "Engine",
            "GASCore",
            "GameplayAbilities", 
            "GameplayTasks", 
            "GameplayTags",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(
        new string[]
        {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore"
        }
        );
    }
}