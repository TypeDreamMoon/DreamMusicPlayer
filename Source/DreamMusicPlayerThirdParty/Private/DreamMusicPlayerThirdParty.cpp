#include "DreamMusicPlayerThirdParty.h"

#include "DreamMusicPlayerLog.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FDreamMusicPlayerThirdPartyModule"

void FDreamMusicPlayerThirdPartyModule::StartupModule()
{
	TArray<FString> DllNames = {
		"ogg.dll",
		"zlib1.dll",
		"FLAC.dll",
		"FLAC++.dll",
		"libaubio-5.dll",
	};
	
	FString BaseDir = IPluginManager::Get().FindPlugin("DreamMusicPlayer")->GetBaseDir();
	FString LibDir = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64"));

	for (const FString& Element : DllNames)
	{
		FString DllPath = FPaths::Combine(*LibDir, Element);
		void* Handle = FPlatformProcess::GetDllHandle(*DllPath);
		LibraryHandles.Add(DllPath, Handle);
		if (!Handle)
		{
			DMP_LOG_CHANNEL(Error, "ThirdParty", "Failed to load %s at %s", *Element, *DllPath)
		}
		else
		{
			DMP_LOG_CHANNEL(Log, "ThirdParty", "Loaded %s at %s", *Element, *DllPath);
		}
	}
}

void FDreamMusicPlayerThirdPartyModule::ShutdownModule()
{
	// 4. 释放句柄 (反向释放)
	for (auto& Element : LibraryHandles)
	{
		if (Element.Value)
		{
			FPlatformProcess::FreeDllHandle(Element.Value);
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDreamMusicPlayerThirdPartyModule, DreamMusicPlayerThirdParty)
