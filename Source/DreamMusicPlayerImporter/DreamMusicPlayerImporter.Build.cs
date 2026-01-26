using UnrealBuildTool;

public class DreamMusicPlayerImporter : ModuleRules
{
    public DreamMusicPlayerImporter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "DreamMusicPlayer",
                "DreamMusicPlayerLyric",
                "DreamMusicPlayerThirdParty",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "EditorSubsystem",
                "ToolMenus",
                "EditorStyle",
                "EditorWidgets",
                "PropertyEditor",
                "AssetTools",
                "ContentBrowser",
                "DesktopPlatform", 
                "EditorScriptingUtilities",
                "InputCore",
            }
        );
    }
}