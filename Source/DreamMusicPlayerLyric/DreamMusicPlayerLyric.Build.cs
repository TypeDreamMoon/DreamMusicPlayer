using System.IO;
using UnrealBuildTool;

public class DreamMusicPlayerLyric : ModuleRules
{
	public DreamMusicPlayerLyric(ReadOnlyTargetRules Target) : base(Target)
	{
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty", "libdlp");
		
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
			}
		);

		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));

		// 链接第三方库
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string LibFolderPath = Path.Combine(ThirdPartyPath, "lib", "amd64");
			PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "libdlp.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibFolderPath, "libdlp_ex.lib"));
		}
	}
}