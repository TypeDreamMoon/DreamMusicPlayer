using UnrealBuildTool;

public class DreamMusicPlayerExpansion : ModuleRules
{
    public DreamMusicPlayerExpansion(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "DreamMusicPlayer",
                "DreamMusicPlayerThirdParty",
                "MediaAssets", 
                "ImgMedia", 
                "AudioSynesthesia",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "AudioSynesthesiaCore",
                "AudioExtensions",
                "AudioAnalyzer",
                "AudioMixer",
            }
        );
    }
}