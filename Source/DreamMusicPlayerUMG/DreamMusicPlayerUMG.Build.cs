using UnrealBuildTool;

public class DreamMusicPlayerUMG : ModuleRules
{
    public DreamMusicPlayerUMG(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UMG",
                "DreamMusicPlayer",
                "DreamMusicPlayerLyric",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
            }
        );
    }
}