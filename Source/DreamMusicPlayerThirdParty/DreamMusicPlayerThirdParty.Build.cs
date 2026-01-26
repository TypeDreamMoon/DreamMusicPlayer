using System.IO;
using UnrealBuildTool;

public class DreamMusicPlayerThirdParty : ModuleRules
{
    public DreamMusicPlayerThirdParty(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "DreamMusicPlayer",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "ImageWrapper",
            }
        );
        
        string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");
        
        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));
        
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string LibFolderPath = Path.Combine(ThirdPartyPath, "lib", "win64");
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "tag.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "zlib.lib"));
        }
        
        PublicDefinitions.Add("TAGLIB_STATIC");

        bEnableExceptions = true;
        bUseRTTI = true;
    }
}