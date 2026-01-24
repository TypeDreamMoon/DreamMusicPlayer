#pragma once

#include "CoreMinimal.h"
#include "DreamLyricTypes.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_Lyric.h"

namespace FDreamLyricUtils
{
	using namespace dlp;

	ELyricContentRole ConvertRole(EDreamMusicLyricTextRole InRole);

	EDreamMusicLyricTextRole ConvertRole(ELyricContentRole InRole);

	EFileFormat ConvertFormat(EDreamMusicPlayerLyricType Format);

	EDreamMusicPlayerLyricType ConvertFormat(EFileFormat Format);

	TArray<FDreamMusicLyricGroup> ReadLyricFile(const dlp::File::FLyricFile& LyricFile, std::optional<std::reference_wrapper<FDreamMusicLyricMetadata>> OptionalMetadata = std::nullopt);

	FDreamMusicLyricGroup GetLyricAtTimestamp(FDreamMusicTimestamp Timestamp, const TArray<FDreamMusicLyricGroup>& Lyrics);
	FString GetLyricFilePath(FString FileName);
	TArray<FString> GetLyricFileNames();
}
