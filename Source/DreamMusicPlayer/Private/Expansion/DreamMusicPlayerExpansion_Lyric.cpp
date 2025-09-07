// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_Lyric.h"

#include "DreamMusicPlayerDebugLog.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_Lyric.h"
#include "LyricParser/DreamLyricParser.h"
#include "LyricParser/DreamMusicPlayerLyricTools.h"
#include "Kismet/KismetMathLibrary.h"

using namespace FDreamMusicPlayerLyricTools;

void UDreamMusicPlayerExpansion_Lyric::InitializeLyricList()
{
	if (!CurrentMusicData.IsValid())
	{
		DMP_LOG_DEBUG_EXPANSION(Error, TEXT("CurrentMusicData is Not Valid"));
		return;
	}
	DMP_LOG_DEBUG_EXPANSION(Log, TEXT("InitializeLyricList - Begin"));
	CurrentMusicLyricList.Empty();

	UDreamMusicPlayerExpansionData_Lyric* ExpansionData = CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_Lyric>();

	FDreamLyricParser Parser(GetLyricFilePath(ExpansionData->LyricFileName),
	                         ExpansionData->LyricParseFileType,
	                         ExpansionData->LyricParseLineType,
	                         ExpansionData->LrcLyricType);

	CurrentMusicLyricList = Parser.GetLyrics();

	OnLyricListChanged.Broadcast(CurrentMusicLyricList);
	DMP_LOG_DEBUG_EXPANSION(Log, TEXT("InitializeLyricList Count : %02d - End"), CurrentMusicLyricList.Num());
}

void UDreamMusicPlayerExpansion_Lyric::PlayMusicWithLyric(FDreamMusicLyric InLyric)
{
	if (CurrentMusicLyricList.Contains(InLyric))
	{
		float Time = InLyric.StartTimestamp.ToSeconds();
		Time = UKismetMathLibrary::NormalizeToRange(Time, 0.0f, MusicPlayerComponent->CurrentMusicDuration);
		MusicPlayerComponent->SetMusicPercent(Time);
	}
	else
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("%hs Lyric Not Found !!!"), __FUNCTION__)
	}
}

FDreamMusicLyricProgress UDreamMusicPlayerExpansion_Lyric::GetCurrentLyricWordProgress(const FDreamMusicLyricTimestamp& InTimestamp) const
{
	return CalculateWordProgress(InTimestamp);
}

FDreamMusicLyricProgress UDreamMusicPlayerExpansion_Lyric::GetCurrentRomanizationProgress(const FDreamMusicLyricTimestamp& InTimestamp) const
{
	return CalculateWordProgress(InTimestamp, true);
}

FDreamMusicLyricProgress UDreamMusicPlayerExpansion_Lyric::GetCurrentLyricLineProgress(const FDreamMusicLyricTimestamp& InTimestamp) const
{
	return CalculateLineProgress(InTimestamp);
}

void UDreamMusicPlayerExpansion_Lyric::BuildWordDurationCache(bool bUseRoma) const
{
	if (bCacheValid && bLastUseRoma == bUseRoma)
	{
		return;
	}

	const TArray<FDreamMusicLyricWord>& Words = bUseRoma ? CurrentLyric.RomanizationWordTimings : CurrentLyric.WordTimings;

	WordDurationPrefixSum.Empty();
	WordDurationPrefixSum.Reserve(Words.Num());

	int32 CumulativeDuration = 0;
	for (const FDreamMusicLyricWord& Word : Words)
	{
		int32 WordDuration = Word.EndTimestamp.ToMilliseconds() - Word.StartTimestamp.ToMilliseconds();
		CumulativeDuration += WordDuration;
		WordDurationPrefixSum.Add(CumulativeDuration);
	}

	bCacheValid = true;
	bLastUseRoma = bUseRoma;
}

void UDreamMusicPlayerExpansion_Lyric::BP_MusicStart_Implementation()
{
	InitializeLyricList();
}

void UDreamMusicPlayerExpansion_Lyric::BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
	SetCurrentLyric(FDreamMusicPlayerLyricTools::GetLyricAtTimestamp(CurrentTimestamp, CurrentMusicLyricList));
}


FDreamMusicLyricProgress UDreamMusicPlayerExpansion_Lyric::CalculateWordProgress(FDreamMusicLyricTimestamp InCurrentTime, bool bUseRoma) const
{
	// 边界检查
	if (CurrentLyric.IsEmpty())
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		return FDreamMusicLyricProgress(0, 0.0f, false, FDreamMusicLyricWord{});
	}

	// 如果当前时间在歌词行开始之前，返回0
	if (InCurrentTime < CurrentLyric.StartTimestamp)
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		return FDreamMusicLyricProgress(0, 0.0f, false, FDreamMusicLyricWord{});
	}

	// 如果当前时间在歌词行结束之后，返回1（完成状态）
	if (InCurrentTime > CurrentLyric.EndTimestamp)
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		return FDreamMusicLyricProgress(-1, 1.0f, false, FDreamMusicLyricWord{});
	}

	// 如果没有单词时间信息，使用行进度
	if (bUseRoma ? CurrentLyric.IsRomanizationWordsEmpty() : CurrentLyric.IsWordsEmpty())
	{
		return CalculateLineProgress(InCurrentTime);
	}

	// 构建缓存
	BuildWordDurationCache(bUseRoma);

	const TArray<FDreamMusicLyricWord>& Words = bUseRoma ? CurrentLyric.RomanizationWordTimings : CurrentLyric.WordTimings;

	// 性能优化：从上次位置开始查找，通常时间是递增的
	int32 StartIndex = 0;
	if (CachedCurrentWordIndex >= 0 && CachedCurrentWordIndex < Words.Num() &&
		InCurrentTime >= LastCalculationTime)
	{
		StartIndex = CachedCurrentWordIndex;
	}

	// 查找当前单词
	int32 CurrentWordIndex = -1;
	for (int32 i = StartIndex; i < Words.Num(); i++)
	{
		const FDreamMusicLyricWord& Word = Words[i];
		if (InCurrentTime >= Word.StartTimestamp && InCurrentTime < Word.EndTimestamp)
		{
			CurrentWordIndex = i;
			break;
		}
	}

	// 如果从缓存位置没找到，从头查找
	if (CurrentWordIndex == -1 && StartIndex > 0)
	{
		for (int32 i = 0; i < StartIndex; i++)
		{
			const FDreamMusicLyricWord& Word = Words[i];
			if (InCurrentTime >= Word.StartTimestamp && InCurrentTime < Word.EndTimestamp)
			{
				CurrentWordIndex = i;
				break;
			}
		}
	}

	// 更新缓存
	CachedCurrentWordIndex = CurrentWordIndex;
	LastCalculationTime = InCurrentTime;

	int32 LineTotalDuration = CurrentLyric.EndTimestamp.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();

	if (CurrentWordIndex >= 0)
	{
		const FDreamMusicLyricWord& CurrentWord = Words[CurrentWordIndex];

		// 使用前缀和快速计算进度
		int32 ProgressToWordStart = (CurrentWordIndex > 0) ? WordDurationPrefixSum[CurrentWordIndex - 1] : 0;
		int32 CurrentWordElapsed = InCurrentTime.ToMilliseconds() - CurrentWord.StartTimestamp.ToMilliseconds();
		int32 TotalProgress = ProgressToWordStart + CurrentWordElapsed;

		float LineProgress = static_cast<float>(TotalProgress) / static_cast<float>(LineTotalDuration);

		return FDreamMusicLyricProgress(CurrentWordIndex, LineProgress, true, CurrentWord);
	}
	else
	{
		// 回退到行进度计算
		int32 Elapsed = InCurrentTime.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();
		float LineProgress = static_cast<float>(Elapsed) / static_cast<float>(LineTotalDuration);

		return FDreamMusicLyricProgress(-1, LineProgress, false, FDreamMusicLyricWord{});
	}
}

FDreamMusicLyricProgress UDreamMusicPlayerExpansion_Lyric::CalculateLineProgress(FDreamMusicLyricTimestamp InCurrentTime) const
{
	if (InCurrentTime < CurrentLyric.StartTimestamp || InCurrentTime > CurrentLyric.EndTimestamp)
	{
		return FDreamMusicLyricProgress(-1, 0.0f, false, FDreamMusicLyricWord{});
	}

	int32 LineDuration = CurrentLyric.EndTimestamp.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();
	int32 Elapsed = InCurrentTime.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();

	// 修复整数除法问题
	float Progress = static_cast<float>(Elapsed) / static_cast<float>(LineDuration);

	return FDreamMusicLyricProgress(-1, Progress, false, FDreamMusicLyricWord{});
}

void UDreamMusicPlayerExpansion_Lyric::SetCurrentLyric(FDreamMusicLyric InLyric)
{
	if (InLyric != CurrentLyric && InLyric.IsNotEmpty())
	{
		ClearLyricProgressCache();
		CurrentLyric = InLyric;
		OnLyricChanged.Broadcast(CurrentLyric, CurrentMusicLyricList.Find(CurrentLyric));
		DMP_LOG_DEBUG_EXPANSION(Log, "Lyric", TEXT("Set : Time : %02d:%02d.%02d Content : %s"),
		                        InLyric.StartTimestamp.Minute, InLyric.StartTimestamp.Seconds, InLyric.StartTimestamp.Millisecond, *InLyric.Content);
	}
}
