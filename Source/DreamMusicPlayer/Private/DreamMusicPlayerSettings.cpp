// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "DreamMusicPlayerSettings.h"

UDreamMusicPlayerSettings::UDreamMusicPlayerSettings()
{
}

UDreamMusicPlayerSettings* UDreamMusicPlayerSettings::Get()
{
	return GetMutableDefault<UDreamMusicPlayerSettings>();
}
