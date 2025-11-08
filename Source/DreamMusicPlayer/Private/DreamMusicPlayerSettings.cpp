// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "DreamMusicPlayerSettings.h"

#include "DreamMusicPlayerVersion.h"

UDreamMusicPlayerSettings::UDreamMusicPlayerSettings()
{
	Versions = FString::Printf(TEXT("Plugin Version : %s Lyric Parser Library Version : %s"),
		*DreamMusicPlayerVersion::PluginVersionName, *DreamMusicPlayerVersion::LyricParserLibraryVersionName);
}

UDreamMusicPlayerSettings* UDreamMusicPlayerSettings::Get()
{
	return GetMutableDefault<UDreamMusicPlayerSettings>();
}
