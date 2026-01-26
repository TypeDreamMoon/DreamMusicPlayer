#pragma once

#include "Logging/LogMacros.h"

DREAMMUSICPLAYER_API DECLARE_LOG_CATEGORY_EXTERN(LogDreamMusicPlayer, All, All);

#define DMP_LOG(V, F, ...) UE_LOG(LogDreamMusicPlayer, V, F, ##__VA_ARGS__)
#define DMP_LOG_CHANNEL(V, Channel, F, ...) UE_LOG(LogDreamMusicPlayer, V, TEXT("[" TEXT(Channel) "]" F), ##__VA_ARGS__)
