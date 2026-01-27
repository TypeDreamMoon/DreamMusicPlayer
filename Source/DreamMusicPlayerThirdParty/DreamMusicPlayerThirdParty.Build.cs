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
                "StylusInputWintab",
                "Slate",
                "SlateCore",
                "ApplicationCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "ImageWrapper",
                "Projects",
                "Json", 
                "ImageWriteQueue", 
                "StylusInput",
                "Slate",
            }
        );
        
        string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");
        
        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));
        
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string LibFolderPath = Path.Combine(ThirdPartyPath, "lib", "win64");
            string BinFolderPath = Path.Combine(ThirdPartyPath, "bin", "win64");
            
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "tag.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "zlib.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "ogg.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "FLACpp.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "FLAC.lib"));
            
            RuntimeDependencies.Add("$(BinaryOutputDir)/FLAC++.dll", Path.Combine(BinFolderPath, "FLAC++.dll"));
            RuntimeDependencies.Add("$(BinaryOutputDir)/FLAC.dll", Path.Combine(BinFolderPath, "FLAC.dll"));
            RuntimeDependencies.Add("$(BinaryOutputDir)/zlib1.dll", Path.Combine(BinFolderPath, "zlib1.dll"));
            RuntimeDependencies.Add("$(BinaryOutputDir)/ogg.dll", Path.Combine(BinFolderPath, "ogg.dll"));
            
            PublicDelayLoadDLLs.Add("FLAC++.dll");
            PublicDelayLoadDLLs.Add("FLAC.dll");
            PublicDelayLoadDLLs.Add("zlib1.dll");
            PublicDelayLoadDLLs.Add("ogg.dll");
            
            // Windows Runtime Libraries

            bEnableExceptions = true;
            bUseUnity = false;
            
            PublicSystemLibraries.AddRange(new string[]
            {
                "shlwapi.lib",
                "runtimeobject.lib",
                "user32.lib",
                "shell32.lib"
            });
            
            PrivateIncludePaths.Add(Path.Combine(
                Target.WindowsPlatform.WindowsSdkDir,        
                "Include", 
                Target.WindowsPlatform.WindowsSdkVersion, 
                "cppwinrt"));
            
            PublicDefinitions.Add("TAGLIB_STATIC");
        
            bUseRTTI = true;
        }
    }
}