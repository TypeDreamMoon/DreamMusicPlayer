using System.IO;
using UnrealBuildTool;

public class DreamMusicPlayerLyric : ModuleRules
{
	public DreamMusicPlayerLyric(ReadOnlyTargetRules Target) : base(Target)
	{
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty", "DreamLyricParser");
		
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
				"ContentBrowser",
				"DesktopPlatform", 
				"EditorScriptingUtilities",
			}
		);

		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));

		// 链接第三方库
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string LibFolderPath = Path.Combine(ThirdPartyPath, "lib", "win64");
            
			// A. 链接静态库 (.lib)
			// 务必确保文件名准确，包含后缀 .lib
			PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "DreamLyricParser.lib"));

			// B. 处理动态库 (.dll) 拷贝
			// 这行代码告诉 UBT：编译时把这个 DLL 拷贝到项目的 Binaries/Win64 目录下
			string DllPath = Path.Combine(LibFolderPath, "DreamLyricParser.dll");
            
			// $(BinaryOutputDir) 是引擎宏，指向最终生成的 exe/dll 所在目录
			RuntimeDependencies.Add("$(BinaryOutputDir)/DreamLyricParser.dll", DllPath);
		}
	}
}