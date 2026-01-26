#pragma once

#include "CoreMinimal.h"
#include "DreamLyricTypes.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_Lyric.h"

struct DREAMMUSICPLAYERLYRIC_API FDreamLyricUtils
{
public:
	static dlp::ELyricContentRole ConvertRole(EDreamMusicLyricTextRole InRole);
	static EDreamMusicLyricTextRole ConvertRole(dlp::ELyricContentRole InRole);

	static dlp::EFileFormat ConvertFormat(EDreamMusicPlayerLyricType Format);
	static EDreamMusicPlayerLyricType ConvertFormat(dlp::EFileFormat Format);

	static TArray<FDreamMusicLyricGroup> ReadLyricFile(const dlp::File::FLyricFile& LyricFile, std::optional<std::reference_wrapper<FDreamMusicLyricMetadata>> OptionalMetadata = std::nullopt);
	static FDreamMusicLyricGroup GetLyricAtTimestamp(FDreamMusicTimestamp Timestamp, const TArray<FDreamMusicLyricGroup>& Lyrics);
	static FString GetLyricFilePath(FString FileName);
	static TArray<FString> GetLyricFileNames();
};
