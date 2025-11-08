using UnrealBuildTool;

public class DreamMusicPlayerEditor : ModuleRules
{
    public DreamMusicPlayerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
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
                "EditorStyle",
                "EditorWidgets",
                "ToolMenus",
                "PropertyEditor",
                "AssetTools",
                "AssetRegistry",
                "ContentBrowser",
                "WorkspaceMenuStructure",
                "UnrealEd",
                "ApplicationCore",
                "InputCore",
                "EditorScriptingUtilities"
            }
        );
    }
}