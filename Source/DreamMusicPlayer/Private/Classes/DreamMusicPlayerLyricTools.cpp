#include "Classes/DreamMusicPlayerLyricTools.h"

#include "DreamMusicPlayerLog.h"
#include "DreamMusicPlayerSettings.h"
#include "Classes/DreamMusicPlayerLyricFileParser.h"

FDreamMusicLyric FDreamMusicPlayerLyricTools::GetLyricAtTimestamp(FDreamMusicLyricTimestamp Timestamp, const TArray<FDreamMusicLyric>& Lyrics)
{
	if (Lyrics.Num() == 0)
	{
		return FDreamMusicLyric::EMPTY();
	}

	int Low = 0;
	int High = Lyrics.Num() - 1;
	int ResultIndex = -1;

	while (Low <= High)
	{
		int Mid = (Low + High) / 2;
		const FDreamMusicLyric& Lyric = Lyrics[Mid];

		if (Lyric.Timestamp == Timestamp)
		{
			return Lyric; // 精确匹配时间戳，返回对应的歌词
		}
		else if (Lyric.Timestamp < Timestamp)
		{
			ResultIndex = Mid; // 记录小于目标时间戳的最大时间戳位置
			Low = Mid + 1; // 时间戳小于目标时间戳，搜索后半部分
		}
		else
		{
			High = Mid - 1; // 时间戳大于目标时间戳，搜索前半部分
		}
	}

	// 返回小于等于目标时间戳的最大时间戳对应的歌词
	if (ResultIndex >= 0)
	{
		return Lyrics[ResultIndex];
	}

	return FDreamMusicLyric::EMPTY();
}

FString FDreamMusicPlayerLyricTools::GetLyricFilePath(FString FileName)
{
	FString LocalPath;
	FPackageName::TryConvertGameRelativePackagePathToLocalPath(GetDefault<UDreamMusicPlayerSettings>()->LyricContentPath.Path, LocalPath);
	LocalPath /= FileName;
	return LocalPath;
}

TArray<FString> FDreamMusicPlayerLyricTools::GetLyricFileNames()
{
	TArray<FString> Names;
	TArray<FString> LrcFiles, AssFiles, SrtFiles;

	FString LongPath;
	FPackageName::TryConvertGameRelativePackagePathToLocalPath(GetDefault<UDreamMusicPlayerSettings>()->LyricContentPath.Path, LongPath);

	IFileManager::Get().FindFiles(LrcFiles, *LongPath, TEXT("*.lrc"));
	IFileManager::Get().FindFiles(AssFiles, *LongPath, TEXT("*.ass"));
	IFileManager::Get().FindFiles(SrtFiles, *LongPath, TEXT("*.srt"));

	Names.Append(LrcFiles);
	Names.Append(AssFiles);
	Names.Append(SrtFiles);

	return Names;
}
