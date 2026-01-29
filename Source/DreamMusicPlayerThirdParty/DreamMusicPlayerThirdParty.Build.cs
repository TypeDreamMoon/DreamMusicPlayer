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
				"Json"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"ImageWrapper",
				"Projects",
				"ImageWriteQueue",
				"StylusInput",
				"Slate",
				"AudioMixer", // LibAubio
				"AudioExtensions", // LibAubio
				"AudioPlatformConfiguration", // LibAubio
			}
		);

		string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");

		PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string LibFolderPath = Path.Combine(ThirdPartyPath, "lib", "win64");
			string BinFolderPath = Path.Combine(ThirdPartyPath, "bin", "win64");

			PublicAdditionalLibraries.AddRange(new string[]
			{
				Path.Combine(LibFolderPath, "tag.lib"),
				Path.Combine(LibFolderPath, "zlib.lib"),
				Path.Combine(LibFolderPath, "ogg.lib"),
				Path.Combine(LibFolderPath, "FLACpp.lib"),
				Path.Combine(LibFolderPath, "FLAC.lib"),
				Path.Combine(LibFolderPath, "aubio.lib"),
			});

			RuntimeDependencies.Add("$(BinaryOutputDir)/FLAC++.dll", Path.Combine(BinFolderPath, "FLAC++.dll"));
			RuntimeDependencies.Add("$(BinaryOutputDir)/FLAC.dll", Path.Combine(BinFolderPath, "FLAC.dll"));
			RuntimeDependencies.Add("$(BinaryOutputDir)/zlib1.dll", Path.Combine(BinFolderPath, "zlib1.dll"));
			RuntimeDependencies.Add("$(BinaryOutputDir)/ogg.dll", Path.Combine(BinFolderPath, "ogg.dll"));
			RuntimeDependencies.Add("$(BinaryOutputDir)/libaubio-5.dll", Path.Combine(BinFolderPath, "libaubio-5.dll"));

			PublicDelayLoadDLLs.Add("FLAC++.dll");
			PublicDelayLoadDLLs.Add("FLAC.dll");
			PublicDelayLoadDLLs.Add("zlib1.dll");
			PublicDelayLoadDLLs.Add("ogg.dll");
			PublicDelayLoadDLLs.Add("libaubio-5.dll");

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
			
			PublicDefinitions.AddRange(new string[]
			{
				"TAGLIB_STATIC",
			});

			bUseRTTI = true;
		}
	}
}