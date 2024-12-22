#include "Classes/DreamMusicPlayerLyricTools.h"

#include "DreamMusicPlayerSettings.h"
#include "DreamMusicPlayerLog.h"

TArray<FDreamMusicLyric> FDreamMusicPlayerLyricTools::LoadLyricFromFile(FString FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		DMP_LOG(Warning, TEXT("Lyric Load Failed. Lyric Path Is Not Valid!"))
		return {};
	}
	TArray<FDreamMusicLyric> Buffer;
	TArray<FString> Lines;
	FFileHelper::LoadFileToStringArray(Lines, *FilePath);
	for (auto Element : Lines)
	{
		Buffer.Add(FDreamMusicLyric(Element));
		DMP_LOG(Log, TEXT("Lyric : Time : %02d:%02d.%02d : Content : %s"), Buffer.Last().Timestamp.Minute,
		        Buffer.Last().Timestamp.Seconds, Buffer.Last().Timestamp.Millisecond, *Buffer.Last().Content);
	}

	return Buffer;
}

FDreamMusicLyric FDreamMusicPlayerLyricTools::GetCurrentLyric(FDreamMusicLyricTimestamp CurrentTime,
                                                              const TArray<FDreamMusicLyric>& Lyrics)
{
	// TODO : 这里的开销可能有些大 需要优化
	for (auto Element : Lyrics)
	{
		if (Element.Timestamp == CurrentTime)
			return Element;
	}
	return FDreamMusicLyric();
}

FDreamMusicLyricTimestamp FDreamMusicPlayerLyricTools::Conv_TimestampFromFloat(float Time)
{
	int M, S, MS;
	Time = FMath::Abs(Time);
	M = FMath::FloorToInt(Time / 60.f);
	S = FMath::FloorToInt(Time - (M * 60.f));
	MS = FMath::FloorToInt((Time - FMath::FloorToFloat(Time)) * 100.f);
	FDreamMusicLyricTimestamp CurrentTimeStamp(M, S, MS);
	return CurrentTimeStamp;
}

float FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(FDreamMusicLyricTimestamp Timestamp)
{
	return Timestamp.Minute * 60.f + Timestamp.Seconds + Timestamp.Millisecond / 100.f;
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
	FString LongPath;
	FPackageName::TryConvertGameRelativePackagePathToLocalPath(GetDefault<UDreamMusicPlayerSettings>()->LyricContentPath.Path, LongPath);
	IFileManager::Get().FindFiles(Names, *LongPath);
	return Names;
}

