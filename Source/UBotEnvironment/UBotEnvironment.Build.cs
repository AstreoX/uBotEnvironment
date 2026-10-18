using UnrealBuildTool;

public class UBotEnvironment : ModuleRules
{
    public UBotEnvironment(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UBotCore"
        });
    }
}
