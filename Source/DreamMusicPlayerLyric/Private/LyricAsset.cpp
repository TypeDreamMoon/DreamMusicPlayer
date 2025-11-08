#include "LyricAsset.h"
#include "Misc/App.h"

// ========== FLyricTimeSpan 实现 ==========

int64 FLyricTimeSpan::ToTotalMilliseconds() const
{
	return static_cast<int64>(Hours) * 3600000LL +
		   static_cast<int64>(Minutes) * 60000LL +
		   static_cast<int64>(Seconds) * 1000LL +
		   static_cast<int64>(Milliseconds);
}

float FLyricTimeSpan::ToSeconds() const
{
	return static_cast<float>(ToTotalMilliseconds()) / 1000.0f;
}

float FLyricTimeSpan::ToMinutes() const
{
	return ToSeconds() / 60.0f;
}

FLyricTimeSpan FLyricTimeSpan::FromMilliseconds(int64 TotalMilliseconds)
{
	FLyricTimeSpan Result;
	Result.Hours = static_cast<int32>(TotalMilliseconds / 3600000LL);
	TotalMilliseconds %= 3600000LL;
	Result.Minutes = static_cast<int32>(TotalMilliseconds / 60000LL);
	TotalMilliseconds %= 60000LL;
	Result.Seconds = static_cast<int32>(TotalMilliseconds / 1000LL);
	Result.Milliseconds = static_cast<int32>(TotalMilliseconds % 1000LL);
	return Result;
}

FLyricTimeSpan FLyricTimeSpan::FromSeconds(float TotalSeconds)
{
	return FromMilliseconds(static_cast<int64>(TotalSeconds * 1000.0f));
}

void FLyricTimeSpan::Normalize()
{
	int64 TotalMs = ToTotalMilliseconds();
	if (TotalMs < 0)
	{
		TotalMs = 0;
	}
	
	*this = FromMilliseconds(TotalMs);
}

FString FLyricTimeSpan::ToString(bool bIncludeHours, int32 FractionalDigits) const
{
	FString Format;
	if (bIncludeHours)
	{
		Format = FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
	}
	else
	{
		Format = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	}

	if (FractionalDigits > 0)
	{
		FString FractionalPart = FString::Printf(TEXT("%03d"), Milliseconds);
		if (FractionalDigits < 3)
		{
			FractionalPart = FractionalPart.Left(FMath::Clamp(FractionalDigits, 0, 3));
		}
		Format += FString::Printf(TEXT(".%s"), *FractionalPart);
	}

	return Format;
}

bool FLyricTimeSpan::IsZero() const
{
	return Hours == 0 && Minutes == 0 && Seconds == 0 && Milliseconds == 0;
}

bool FLyricTimeSpan::operator==(const FLyricTimeSpan& Other) const
{
	return ToTotalMilliseconds() == Other.ToTotalMilliseconds();
}

bool FLyricTimeSpan::operator!=(const FLyricTimeSpan& Other) const
{
	return !(*this == Other);
}

bool FLyricTimeSpan::operator<(const FLyricTimeSpan& Other) const
{
	return ToTotalMilliseconds() < Other.ToTotalMilliseconds();
}

bool FLyricTimeSpan::operator<=(const FLyricTimeSpan& Other) const
{
	return ToTotalMilliseconds() <= Other.ToTotalMilliseconds();
}

bool FLyricTimeSpan::operator>(const FLyricTimeSpan& Other) const
{
	return ToTotalMilliseconds() > Other.ToTotalMilliseconds();
}

bool FLyricTimeSpan::operator>=(const FLyricTimeSpan& Other) const
{
	return ToTotalMilliseconds() >= Other.ToTotalMilliseconds();
}

FLyricTimeSpan FLyricTimeSpan::operator+(const FLyricTimeSpan& Other) const
{
	return FromMilliseconds(ToTotalMilliseconds() + Other.ToTotalMilliseconds());
}

FLyricTimeSpan FLyricTimeSpan::operator-(const FLyricTimeSpan& Other) const
{
	int64 Result = ToTotalMilliseconds() - Other.ToTotalMilliseconds();
	return FromMilliseconds(FMath::Max<int64>(0, Result));
}

// ========== FLyricWord 实现 ==========

int64 FLyricWord::GetDurationMilliseconds() const
{
	if (!bHasEndTime)
	{
		return 0;
	}
	return EndTime.ToTotalMilliseconds() - StartTime.ToTotalMilliseconds();
}

bool FLyricWord::IsTimeInRange(const FLyricTimeSpan& Time) const
{
	if (Time < StartTime)
	{
		return false;
	}
	if (!bHasEndTime)
	{
		return true; // 如果没有结束时间，只要时间 >= 开始时间就认为在范围内
	}
	return Time <= EndTime;
}

bool FLyricWord::IsEmpty() const
{
	return Text.IsEmpty();
}

// ========== FLyricLine 实现 ==========

FLyricTimeSpan FLyricLine::GetStartTime() const
{
	if (Words.Num() == 0)
	{
		return FLyricTimeSpan();
	}
	return Words[0].StartTime;
}

FLyricTimeSpan FLyricLine::GetEndTime() const
{
	if (Words.Num() == 0)
	{
		return FLyricTimeSpan();
	}
	const FLyricWord& LastWord = Words[Words.Num() - 1];
	if (LastWord.bHasEndTime)
	{
		return LastWord.EndTime;
	}
	return LastWord.StartTime;
}

bool FLyricLine::IsTimeInRange(const FLyricTimeSpan& Time) const
{
	if (Words.Num() == 0)
	{
		return false;
	}
	FLyricTimeSpan Start = GetStartTime();
	FLyricTimeSpan End = GetEndTime();
	return Time >= Start && Time <= End;
}

bool FLyricLine::IsEmpty() const
{
	return Text.IsEmpty() && Words.Num() == 0;
}

// ========== FLyricGroup 实现 ==========

FLyricLine* FLyricGroup::GetLineByRole(ELyricTextRole Role)
{
	for (FLyricLine& Line : Lines)
	{
		if (Line.Role == Role)
		{
			return &Line;
		}
	}
	return nullptr;
}

const FLyricLine* FLyricGroup::GetLineByRole(ELyricTextRole Role) const
{
	for (const FLyricLine& Line : Lines)
	{
		if (Line.Role == Role)
		{
			return &Line;
		}
	}
	return nullptr;
}

FLyricLine* FLyricGroup::GetMainLyricLine()
{
	// 优先返回 Lyric 角色
	FLyricLine* LyricLine = GetLineByRole(ELyricTextRole::Lyric);
	if (LyricLine)
	{
		return LyricLine;
	}
	// 如果没有 Lyric，返回第一行
	if (Lines.Num() > 0)
	{
		return &Lines[0];
	}
	return nullptr;
}

bool FLyricGroup::IsTimeMatch(const FLyricTimeSpan& Time, float ToleranceSeconds) const
{
	float TimeSeconds = Time.ToSeconds();
	float TimestampSeconds = Timestamp.ToSeconds();
	float Diff = FMath::Abs(TimeSeconds - TimestampSeconds);
	return Diff <= ToleranceSeconds;
}

bool FLyricGroup::IsEmpty() const
{
	return Lines.Num() == 0;
}

// ========== FLyricMetadata 实现 ==========

FString FLyricMetadata::GetValue(const FString& Key, const FString& DefaultValue) const
{
	const FString* FoundValue = Items.Find(Key);
	return FoundValue ? *FoundValue : DefaultValue;
}

void FLyricMetadata::SetValue(const FString& Key, const FString& Value)
{
	Items.Add(Key, Value);
}

bool FLyricMetadata::HasKey(const FString& Key) const
{
	return Items.Contains(Key);
}

// ========== ULyricAsset 实现 ==========

ULyricAsset::ULyricAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

int32 ULyricAsset::FindGroupByTime(const FLyricTimeSpan& Time, float ToleranceSeconds) const
{
	float TimeSeconds = Time.ToSeconds();
	
	for (int32 i = 0; i < Groups.Num(); ++i)
	{
		if (Groups[i].IsTimeMatch(Time, ToleranceSeconds))
		{
			return i;
		}
	}
	
	return INDEX_NONE;
}

FLyricGroup ULyricAsset::GetGroupByTime(const FLyricTimeSpan& Time, float ToleranceSeconds)
{
	int32 Index = FindGroupByTime(Time, ToleranceSeconds);
	if (Index != INDEX_NONE)
	{
		return Groups[Index];
	}
	return FLyricGroup();
}

TArray<FLyricLine> ULyricAsset::GetLinesByRole(ELyricTextRole Role) const
{
	TArray<FLyricLine> Result;
	for (const FLyricGroup& Group : Groups)
	{
		for (const FLyricLine& Line : Group.Lines)
		{
			if (Line.Role == Role)
			{
				Result.Add(Line);
			}
		}
	}
	return Result;
}

FLyricAssetStatistics ULyricAsset::GetStatistics() const
{
	FLyricAssetStatistics Stats;
	Stats.TotalGroups = Groups.Num();
	
	FLyricTimeSpan MinTime = FLyricTimeSpan::FromMilliseconds(INT64_MAX);
	FLyricTimeSpan MaxTime = FLyricTimeSpan();
	
	bool bHasWords = false;
	TSet<ELyricTextRole> Roles;
	
	for (const FLyricGroup& Group : Groups)
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
		
		for (const FLyricLine& Line : Group.Lines)
		{
			Roles.Add(Line.Role);
			Stats.TotalWords += Line.Words.Num();
			
			if (Line.Words.Num() > 0)
			{
				bHasWords = true;
			}
			
			// 检查是否有逐词时间信息
			for (const FLyricWord& Word : Line.Words)
			{
				if (Word.bHasEndTime)
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

float ULyricAsset::GetTotalDurationSeconds() const
{
	if (Groups.Num() == 0)
	{
		return 0.0f;
	}
	
	FLyricTimeSpan MinTime = Groups[0].Timestamp;
	FLyricTimeSpan MaxTime = Groups[0].Timestamp;
	
	for (const FLyricGroup& Group : Groups)
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

FLyricTimeSpan ULyricAsset::GetTotalDuration() const
{
	return FLyricTimeSpan::FromSeconds(GetTotalDurationSeconds());
}

int32 ULyricAsset::GetTotalLineCount() const
{
	int32 Count = 0;
	for (const FLyricGroup& Group : Groups)
	{
		Count += Group.Lines.Num();
	}
	return Count;
}

int32 ULyricAsset::GetTotalWordCount() const
{
	int32 Count = 0;
	for (const FLyricGroup& Group : Groups)
	{
		for (const FLyricLine& Line : Group.Lines)
		{
			Count += Line.Words.Num();
		}
	}
	return Count;
}

void ULyricAsset::SortGroupsByTime()
{
	Groups.Sort([](const FLyricGroup& A, const FLyricGroup& B)
	{
		return A.Timestamp < B.Timestamp;
	});
}

bool ULyricAsset::Validate() const
{
	return GetValidationErrors().Num() == 0;
}

TArray<FString> ULyricAsset::GetValidationErrors() const
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
		const FLyricGroup& Group = Groups[i];
		
		if (Group.Lines.Num() == 0)
		{
			Errors.Add(FString::Printf(TEXT("组 %d 不包含任何行"), i));
		}
		
		// 检查每行
		for (int32 j = 0; j < Group.Lines.Num(); ++j)
		{
			const FLyricLine& Line = Group.Lines[j];
			
			if (Line.IsEmpty())
			{
				Errors.Add(FString::Printf(TEXT("组 %d 行 %d 为空"), i, j));
			}
			
			// 检查单词时间
			for (int32 k = 0; k < Line.Words.Num() - 1; ++k)
			{
				if (Line.Words[k].bHasEndTime && Line.Words[k].EndTime > Line.Words[k + 1].StartTime)
				{
					Errors.Add(FString::Printf(TEXT("组 %d 行 %d 单词 %d 的结束时间晚于下一个单词的开始时间"), i, j, k));
				}
			}
		}
	}
	
	return Errors;
}

void ULyricAsset::Clear()
{
	Groups.Empty();
	Metadata.Items.Empty();
	SourceFileName.Empty();
}

bool ULyricAsset::IsEmpty() const
{
	return Groups.Num() == 0;
}

bool ULyricAsset::HasWordTimings() const
{
	for (const FLyricGroup& Group : Groups)
	{
		for (const FLyricLine& Line : Group.Lines)
		{
			for (const FLyricWord& Word : Line.Words)
			{
				if (Word.bHasEndTime)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool ULyricAsset::HasMultipleRoles() const
{
	TSet<ELyricTextRole> Roles;
	for (const FLyricGroup& Group : Groups)
	{
		for (const FLyricLine& Line : Group.Lines)
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
FString ULyricAsset::GetDesc()
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
