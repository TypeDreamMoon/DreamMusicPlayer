#pragma once

#include "DreamMusicPlayerCommon.h"

namespace FDreamMusicPlayerLyricTools
{
	DREAMMUSICPLAYER_API TArray<FDreamMusicLyric> LoadLyricFromFile(FString FilePath);
	DREAMMUSICPLAYER_API FDreamMusicLyric GetCurrentLyric(FDreamMusicLyricTimestamp CurrentTime, const TArray<FDreamMusicLyric>& Lyrics);
	DREAMMUSICPLAYER_API FDreamMusicLyricTimestamp Conv_TimestampFromFloat(float Time);
	DREAMMUSICPLAYER_API float Conv_FloatFromTimestamp(FDreamMusicLyricTimestamp Timestamp);
	DREAMMUSICPLAYER_API FString GetLyricFilePath(FString FileName);
	DREAMMUSICPLAYER_API TArray<FString> GetLyricFileNames();
}
