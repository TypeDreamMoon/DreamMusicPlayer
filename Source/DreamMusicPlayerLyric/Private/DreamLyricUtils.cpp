#include "DreamLyricUtils.h"

#include "DreamMusicPlayerSettings.h"

FDreamMusicLyricGroup FDreamLyricUtils::GetLyricAtTimestamp(FDreamMusicTimestamp Timestamp, const TArray<FDreamMusicLyricGroup>& Lyrics)
{
	if (Lyrics.Num() == 0)
	{
		return {};
	}

	int Low = 0;
	int High = Lyrics.Num() - 1;
	int ResultIndex = -1;

	while (Low <= High)
	{
		int Mid = (Low + High) / 2;
		const FDreamMusicLyricGroup& Lyric = Lyrics[Mid];

		if (Lyric.StartTimestamp == Timestamp)
		{
			return Lyric; // 精确匹配时间戳，返回对应的歌词
		}
		else if (Lyric.StartTimestamp < Timestamp)
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

	return {};
}

FString FDreamLyricUtils::GetLyricFilePath(FString FileName)
{
	FString LocalPath;
	FPackageName::TryConvertGameRelativePackagePathToLocalPath(GetDefault<UDreamMusicPlayerSettings>()->LyricContentPath.Path, LocalPath);
	LocalPath /= FileName;
	return LocalPath;
}

TArray<FString> FDreamLyricUtils::GetLyricFileNames()
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

using namespace dlp;

dlp::ELyricContentRole FDreamLyricUtils::ConvertRole(EDreamMusicLyricTextRole InRole)
{
	switch (InRole)
	{
	case EDreamMusicLyricTextRole::None:
		return ELyricContentRole::None;
	case EDreamMusicLyricTextRole::Lyric:
		return ELyricContentRole::Lyric;
	case EDreamMusicLyricTextRole::Translation:
		return ELyricContentRole::Translation;
	case EDreamMusicLyricTextRole::Romanization:
		return ELyricContentRole::Romanization;
	}

	return ELyricContentRole::None;
}

EDreamMusicLyricTextRole FDreamLyricUtils::ConvertRole(dlp::ELyricContentRole InRole)
{
	switch (InRole)
	{
	case ELyricContentRole::None:
		return EDreamMusicLyricTextRole::None;
	case ELyricContentRole::Lyric:
		return EDreamMusicLyricTextRole::Lyric;
	case ELyricContentRole::Translation:
		return EDreamMusicLyricTextRole::Translation;
	case ELyricContentRole::Romanization:
		return EDreamMusicLyricTextRole::Romanization;
	}

	return EDreamMusicLyricTextRole::None;
}

dlp::EFileFormat FDreamLyricUtils::ConvertFormat(EDreamMusicPlayerLyricType Format)
{
	switch (Format)
	{
	case EDreamMusicPlayerLyricType::LRC:
		return EFileFormat::LRC;
	case EDreamMusicPlayerLyricType::ASS:
		return EFileFormat::ASS;
	case EDreamMusicPlayerLyricType::ESLyric:
		return EFileFormat::ESLyric;
	case EDreamMusicPlayerLyricType::SRT:
		return EFileFormat::SRT;
	}

	return EFileFormat::LRC;
}

EDreamMusicPlayerLyricType FDreamLyricUtils::ConvertFormat(dlp::EFileFormat Format)
{
	switch (Format)
	{
	case EFileFormat::LRC:
		return EDreamMusicPlayerLyricType::LRC;
	case EFileFormat::ASS:
		return EDreamMusicPlayerLyricType::ASS;
	case EFileFormat::ESLyric:
		return EDreamMusicPlayerLyricType::ESLyric;
	case EFileFormat::SRT:
		return EDreamMusicPlayerLyricType::SRT;
	}

	return EDreamMusicPlayerLyricType::LRC;
}

TArray<FDreamMusicLyricGroup> FDreamLyricUtils::ReadLyricFile(const dlp::File::FLyricFile& LyricFile, std::optional<std::reference_wrapper<FDreamMusicLyricMetadata>> OptionalMetadata)
{
	if (OptionalMetadata.has_value())
	{
		for (const auto& Metadata : LyricFile.metadata.metadata)
		{
			OptionalMetadata.value().get().Items.Add(Metadata.first.c_str(), Metadata.second.c_str());
		}
	}

	TArray<FDreamMusicLyricGroup> LyricGroups;
	for (const auto& Group : LyricFile.groups)
	{
		FDreamMusicLyricGroup CurrentGroup;
		CurrentGroup.StartTimestamp = Group.group_time_start;
		CurrentGroup.EndTimestamp = Group.group_time_end;
		for (const auto& Lyric : Group.parsed_lines)
		{
			FDreamMusicLyricLine Line;
			Line.Role = ConvertRole(Lyric.role);
			Line.Text = Lyric.lyric.c_str();

			for (const auto& Word : Lyric.parsed_words)
			{
				FDreamMusicLyricWord CurrentWord;
				CurrentWord.StartTimestamp = Word.time_start;
				CurrentWord.EndTimestamp = Word.time_end;
				CurrentWord.Content = Word.word.c_str();

				Line.Words.Add(CurrentWord);
			}

			CurrentGroup.Lines.Add(Line);
		}
	}

	return LyricGroups;
}
