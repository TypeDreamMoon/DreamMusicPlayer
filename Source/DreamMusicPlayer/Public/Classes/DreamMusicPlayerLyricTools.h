#pragma once

#include "DreamMusicPlayerCommon.h"


struct FDreamMusicPlayerLyricFileParserBase;

namespace FDreamMusicPlayerLyricTools
{
	DREAMMUSICPLAYER_API FDreamMusicLyric GetLyricAtTimestamp(FDreamMusicLyricTimestamp Timestamp, const TArray<FDreamMusicLyric>& Lyrics);
	DREAMMUSICPLAYER_API FString GetLyricFilePath(FString FileName);
	DREAMMUSICPLAYER_API TArray<FString> GetLyricFileNames();
}