#pragma once

#include "DreamMusicPlayerLog.h"
#include "DreamMusicPlayerLogChannels.h"
#include "DreamMusicPlayerSettings.h"

#define DMP_SETTING_IS_VALID (IsValid(UDreamMusicPlayerSettings::Get()))

#define DMP_IS_DEBUG DMP_SETTING_IS_VALID ? UDreamMusicPlayerSettings::Get()->bEnableDebugMode : false
#define DMP_IS_DEBUG_TICK DMP_SETTING_IS_VALID ? UDreamMusicPlayerSettings::Get()->bEnableTickDebugMode && DMP_IS_DEBUG : false
#define DMP_IS_DEBUG_EXPANSION DMP_SETTING_IS_VALID ? UDreamMusicPlayerSettings::Get()->bEnableDebugExpansionMode && DMP_IS_DEBUG : false

#define DMP_LOG_DEBUG(V, Channel, Fmt, ...) if (DMP_IS_DEBUG) DMP_LOG(V, TEXT("[" TEXT(Channel) "]" Fmt), ##__VA_ARGS__)
#define DMP_LOG_DEBUG_CHANNEL(V, F, ...) DMP_LOG_DEBUG(V, TEXT("["TEXT(DMP_DEBUG_CHANNEL)"]"), F, ##__VA_ARGS__)

// Debug

#define DMP_LOG_DEBUG_EXPANSION(V, F, ...)	if (DMP_IS_DEBUG_EXPANSION)		DMP_LOG(V, TEXT("[Expansion][%hs]" F), __FUNCTION__, ##__VA_ARGS__)
#define DMP_LOG_DEBUG_TICK(V, F, ...)		if (DMP_IS_DEBUG_TICK)			DMP_LOG(V, TEXT("[Tick]" F), ##__VA_ARGS__)

// Lifetime Debug

#define DMP_LOG_LIFETIME_START		DMP_LOG_DEBUG(Log, DMP_LOG_CHANNEL_LIFETIME,		TEXT("lifetime: start Func: %hs"), __FUNCTION__)
#define DMP_LOG_LIFETIME_END		DMP_LOG_DEBUG(Log, DMP_LOG_CHANNEL_LIFETIME,		TEXT("lifetime: end   Func: %hs"), __FUNCTION__)
