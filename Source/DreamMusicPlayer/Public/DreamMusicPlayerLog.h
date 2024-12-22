#pragma once

#include "Logging/LogMacros.h"

DREAMMUSICPLAYER_API DECLARE_LOG_CATEGORY_EXTERN(LogDreamMusicPlayer, All, All);

#define DMP_LOG(V, F, ...) UE_LOG(LogDreamMusicPlayer, V, F, ##__VA_ARGS__)