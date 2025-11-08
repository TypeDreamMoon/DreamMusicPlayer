using System.IO;
using UnrealBuildTool;

public class DreamMusicPlayerLyric : ModuleRules
{
	private string ThirdPartyPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "ThirdParty")); }
	}

	public DreamMusicPlayerLyric(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"InputCore",
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
				"ContentBrowser"
			}
		);

		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));
		
		// 链接第三方库
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// 直接链接 .lib 文件
			PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "lib", "windows_x64", "DreamLyricParser.lib"));
			
			// 将 DLL 复制到输出目录，确保运行时可以找到
			string DllPath = Path.Combine(ThirdPartyPath, "lib", "windows_x64", "DreamLyricParser.dll");
			if (File.Exists(DllPath))
			{
				RuntimeDependencies.Add(DllPath);
			}
		}
	}
}