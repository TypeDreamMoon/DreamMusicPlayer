#include "DreamLyricAsset.h"
#include "DreamLyricAssetConverter.h"
#include "DreamMusicPlayerCommon.h"
#include "Misc/App.h"

// ========== ULyricAsset 实现 ==========

UDreamLyricAsset::UDreamLyricAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

int32 UDreamLyricAsset::FindGroupByTime(const FDreamMusicLyricTimestamp& Time, float ToleranceSeconds) const
{
	for (int32 i = 0; i < Groups.Num(); ++i)
	{
		if (Groups[i].IsTimeMatch(Time, ToleranceSeconds))
		{
			return i;
		}
	}
	
	return INDEX_NONE;
}

FDreamMusicLyricGroup UDreamLyricAsset::GetGroupByTime(const FDreamMusicLyricTimestamp& Time, float ToleranceSeconds)
{
	int32 Index = FindGroupByTime(Time, ToleranceSeconds);
	if (Index != INDEX_NONE)
	{
		return Groups[Index];
	}
	return FDreamMusicLyricGroup();
}

TArray<FDreamMusicLyricLine> UDreamLyricAsset::GetLinesByRole(EDreamMusicLyricTextRole Role) const
{
	TArray<FDreamMusicLyricLine> Result;
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			if (Line.Role == Role)
			{
				Result.Add(Line);
			}
		}
	}
	return Result;
}

FLyricAssetStatistics UDreamLyricAsset::GetStatistics() const
{
	FLyricAssetStatistics Stats;
	Stats.TotalGroups = Groups.Num();
	
	FDreamMusicLyricTimestamp MinTime = FDreamMusicLyricTimestamp::FromTotalMilliseconds(INT64_MAX);
	FDreamMusicLyricTimestamp MaxTime = FDreamMusicLyricTimestamp();
	
	bool bHasWords = false;
	TSet<EDreamMusicLyricTextRole> Roles;
	
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		Stats.TotalLines += Group.Lines.Num();
		
		if (Group.Timestamp < MinTime)
		{
			MinTime = Group.Timestamp;
		}
		if (Group.Timestamp > MaxTime)
		{
			MaxTime = Group.Timestamp;
		}
		
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			Roles.Add(Line.Role);
			Stats.TotalWords += Line.Words.Num();
			
			if (Line.Words.Num() > 0)
			{
				bHasWords = true;
			}
			
			// 检查是否有逐词时间信息
			for (const FDreamMusicLyricWord& Word : Line.Words)
			{
				if (Word.bHasEndTimestamp)
				{
					Stats.bHasWordTimings = true;
					break;
				}
			}
		}
	}
	
	Stats.StartTime = MinTime;
	Stats.EndTime = MaxTime;
	Stats.TotalDurationSeconds = (MaxTime - MinTime).ToSeconds();
	Stats.bHasMultipleRoles = Roles.Num() > 1;
	
	return Stats;
}

float UDreamLyricAsset::GetTotalDurationSeconds() const
{
	if (Groups.Num() == 0)
	{
		return 0.0f;
	}
	
	FDreamMusicLyricTimestamp MinTime = Groups[0].Timestamp;
	FDreamMusicLyricTimestamp MaxTime = Groups[0].Timestamp;
	
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		if (Group.Timestamp < MinTime)
		{
			MinTime = Group.Timestamp;
		}
		if (Group.Timestamp > MaxTime)
		{
			MaxTime = Group.Timestamp;
		}
	}
	
	return (MaxTime - MinTime).ToSeconds();
}

FDreamMusicLyricTimestamp UDreamLyricAsset::GetTotalDuration() const
{
	return FDreamMusicLyricTimestamp::FromSecondsStatic(GetTotalDurationSeconds());
}

int32 UDreamLyricAsset::GetTotalLineCount() const
{
	int32 Count = 0;
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		Count += Group.Lines.Num();
	}
	return Count;
}

int32 UDreamLyricAsset::GetTotalWordCount() const
{
	int32 Count = 0;
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			Count += Line.Words.Num();
		}
	}
	return Count;
}

void UDreamLyricAsset::SortGroupsByTime()
{
	Groups.Sort([](const FDreamMusicLyricGroup& A, const FDreamMusicLyricGroup& B)
	{
		return A.Timestamp < B.Timestamp;
	});
}

bool UDreamLyricAsset::Validate() const
{
	return GetValidationErrors().Num() == 0;
}

TArray<FString> UDreamLyricAsset::GetValidationErrors() const
{
	TArray<FString> Errors;
	
	if (Groups.Num() == 0)
	{
		Errors.Add(TEXT("歌词资产不包含任何组"));
		return Errors;
	}
	
	// 检查时间戳是否按顺序排列
	for (int32 i = 0; i < Groups.Num() - 1; ++i)
	{
		if (Groups[i].Timestamp > Groups[i + 1].Timestamp)
		{
			Errors.Add(FString::Printf(TEXT("组 %d 和 %d 的时间戳顺序不正确"), i, i + 1));
		}
	}
	
	// 检查每个组
	for (int32 i = 0; i < Groups.Num(); ++i)
	{
		const FDreamMusicLyricGroup& Group = Groups[i];
		
		if (Group.Lines.Num() == 0)
		{
			Errors.Add(FString::Printf(TEXT("组 %d 不包含任何行"), i));
		}
		
		// 检查每行
		for (int32 j = 0; j < Group.Lines.Num(); ++j)
		{
			const FDreamMusicLyricLine& Line = Group.Lines[j];
			
			if (Line.IsEmpty())
			{
				Errors.Add(FString::Printf(TEXT("组 %d 行 %d 为空"), i, j));
			}
			
			// 检查单词时间
			for (int32 k = 0; k < Line.Words.Num() - 1; ++k)
			{
				if (Line.Words[k].bHasEndTimestamp && Line.Words[k].EndTimestamp > Line.Words[k + 1].StartTimestamp)
				{
					Errors.Add(FString::Printf(TEXT("组 %d 行 %d 单词 %d 的结束时间晚于下一个单词的开始时间"), i, j, k));
				}
			}
		}
	}
	
	return Errors;
}

void UDreamLyricAsset::Clear()
{
	Groups.Empty();
	Metadata.Items.Empty();
	SourceFileName.Empty();
}

bool UDreamLyricAsset::IsEmpty() const
{
	return Groups.Num() == 0;
}

bool UDreamLyricAsset::HasWordTimings() const
{
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			for (const FDreamMusicLyricWord& Word : Line.Words)
			{
				if (Word.bHasEndTimestamp)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool UDreamLyricAsset::HasMultipleRoles() const
{
	TSet<EDreamMusicLyricTextRole> Roles;
	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			Roles.Add(Line.Role);
			if (Roles.Num() > 1)
			{
				return true;
			}
		}
	}
	return false;
}

#if WITH_EDITOR
FString UDreamLyricAsset::GetDesc()
{
	FLyricAssetStatistics Stats = GetStatistics();
	FString Title = GetTitle();
	FString Artist = GetArtist();
	
	FString Desc = FString::Printf(
		TEXT("Groups: %d | Lines: %d | Words: %d | Duration: %.2fs"),
		Stats.TotalGroups,
		Stats.TotalLines,
		Stats.TotalWords,
		Stats.TotalDurationSeconds
	);
	
	if (!Title.IsEmpty() || !Artist.IsEmpty())
	{
		Desc += TEXT(" | ");
		if (!Artist.IsEmpty())
		{
			Desc += Artist;
		}
		if (!Title.IsEmpty())
		{
			if (!Artist.IsEmpty())
			{
				Desc += TEXT(" - ");
			}
			Desc += Title;
		}
	}
	
	return Desc;
}
#endif

TArray<FDreamMusicLyric> UDreamLyricAsset::ToLegacyLyrics() const
{
	return ULyricAssetConverter::ConvertAssetToLyrics(this);
}

UDreamLyricAsset* UDreamLyricAsset::FromLegacyLyrics(const TArray<FDreamMusicLyric>& LegacyLyrics, UObject* Outer)
{
	return ULyricAssetConverter::ConvertLyricsToAsset(LegacyLyrics, Outer);
}
