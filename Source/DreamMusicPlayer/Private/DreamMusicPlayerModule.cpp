// Copyright Epic Games, Inc. All Rights Reserved.

#include "DreamMusicPlayerModule.h"

#include "DreamMusicPlayerSettings.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif
#define LOCTEXT_NAMESPACE "FDreamMusicPlayerModule"

void FDreamMusicPlayerModule::StartupModule()
{
#if WITH_EDITOR
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Register Setting Page
		SettingsModule->RegisterSettings(
			"Project",
			"DreamPlugin",
			"DreamMusicPlayerSetting",
			LOCTEXT("DreamMusicPlayerSetting", "DreamMusicPlayerSetting"),
			LOCTEXT("DreamMusicPlayerSetting", "DreamMusicPlayerSetting"),
			GetMutableDefault<UDreamMusicPlayerSettings>()
		);
	}
#endif
}

void FDreamMusicPlayerModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDreamMusicPlayerModule, DreamMusicPlayer)
