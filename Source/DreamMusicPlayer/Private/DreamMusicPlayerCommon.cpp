#include "DreamMusicPlayerCommon.h"

#include "Classes/DreamMusicPlayerExpansionData.h"

FDreamMusicLyricTimestamp::FDreamMusicLyricTimestamp(float InSeconds)
{
	FromSeconds(InSeconds);
}

bool FDreamMusicLyricTimestamp::operator==(const FDreamMusicLyricTimestamp& Target) const
{
	return Target.ToMilliseconds() == ToMilliseconds();
}

bool FDreamMusicLyricTimestamp::operator>=(const FDreamMusicLyricTimestamp& Target) const
{
	return ToMilliseconds() >= Target.ToMilliseconds();
}

bool FDreamMusicLyricTimestamp::operator>(const FDreamMusicLyricTimestamp& Target) const
{
	return ToMilliseconds() > Target.ToMilliseconds();
}

bool FDreamMusicLyricTimestamp::operator<=(const FDreamMusicLyricTimestamp& Target) const
{
	return ToMilliseconds() <= Target.ToMilliseconds();
}

bool FDreamMusicLyricTimestamp::operator<(const FDreamMusicLyricTimestamp& Target) const
{
	return ToMilliseconds() < Target.ToMilliseconds();
}

bool FDreamMusicLyricTimestamp::IsApproximatelyEqual(const FDreamMusicLyricTimestamp& Target, int ToleranceMilliseconds) const
{
	int Diff = ToMilliseconds() - Target.ToMilliseconds();
	return FMath::Abs(Diff) <= ToleranceMilliseconds;
}

const FDreamMusicLyricTimestamp* FDreamMusicLyricTimestamp::FromSeconds(float InSeconds)
{
	Seconds = FMath::FloorToInt(InSeconds);
	Millisecond = FMath::RoundToInt((InSeconds - Seconds) * 1000);
	Hours = Seconds / 3600;
	Seconds %= 3600;
	Minute = Seconds / 60;
	Seconds = Seconds % 60;
	return this;
}

int FDreamMusicLyricTimestamp::ToMilliseconds() const
{
	return Hours * 3600000 + Minute * 60000 + Seconds * 1000 + Millisecond;
}

float FDreamMusicLyricTimestamp::ToSeconds() const
{
	float TotalSeconds = Hours * 3600.0f;
	TotalSeconds += Minute * 60.0f;
	TotalSeconds += Seconds;
	TotalSeconds += Millisecond / 1000.0f;

	return TotalSeconds;
}

int64 FDreamMusicLyricTimestamp::ToTotalMilliseconds() const
{
	return static_cast<int64>(Hours) * 3600000LL +
		static_cast<int64>(Minute) * 60000LL +
		static_cast<int64>(Seconds) * 1000LL +
		static_cast<int64>(Millisecond);
}

FDreamMusicLyricTimestamp FDreamMusicLyricTimestamp::FromTotalMilliseconds(int64 TotalMilliseconds)
{
	FDreamMusicLyricTimestamp Result;
	Result.Hours = static_cast<int32>(TotalMilliseconds / 3600000LL);
	TotalMilliseconds %= 3600000LL;
	Result.Minute = static_cast<int32>(TotalMilliseconds / 60000LL);
	TotalMilliseconds %= 60000LL;
	Result.Seconds = static_cast<int32>(TotalMilliseconds / 1000LL);
	Result.Millisecond = static_cast<int32>(TotalMilliseconds % 1000LL);
	return Result;
}

FDreamMusicLyricTimestamp FDreamMusicLyricTimestamp::FromSecondsStatic(float TotalSeconds)
{
	return FromTotalMilliseconds(static_cast<int64>(TotalSeconds * 1000.0f));
}

void FDreamMusicLyricTimestamp::Normalize()
{
	int64 TotalMs = ToTotalMilliseconds();
	if (TotalMs < 0)
	{
		TotalMs = 0;
	}

	*this = FromTotalMilliseconds(TotalMs);
}

FString FDreamMusicLyricTimestamp::ToStringFormatted(bool bIncludeHours, int32 FractionalDigits) const
{
	FString Format;
	if (bIncludeHours)
	{
		Format = FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minute, Seconds);
	}
	else
	{
		Format = FString::Printf(TEXT("%02d:%02d"), Minute, Seconds);
	}

	if (FractionalDigits > 0)
	{
		FString FractionalPart = FString::Printf(TEXT("%03d"), Millisecond);
		if (FractionalDigits < 3)
		{
			FractionalPart = FractionalPart.Left(FMath::Clamp(FractionalDigits, 0, 3));
		}
		Format += FString::Printf(TEXT(".%s"), *FractionalPart);
	}

	return Format;
}

bool FDreamMusicLyricTimestamp::IsZero() const
{
	return Hours == 0 && Minute == 0 && Seconds == 0 && Millisecond == 0;
}

FDreamMusicLyricTimestamp FDreamMusicLyricTimestamp::operator+(const FDreamMusicLyricTimestamp& Other) const
{
	return FromTotalMilliseconds(ToTotalMilliseconds() + Other.ToTotalMilliseconds());
}

FDreamMusicLyricTimestamp FDreamMusicLyricTimestamp::operator-(const FDreamMusicLyricTimestamp& Other) const
{
	int64 Result = ToTotalMilliseconds() - Other.ToTotalMilliseconds();
	return FromTotalMilliseconds(FMath::Max<int64>(0, Result));
}

FDreamMusicLyricWord::FDreamMusicLyricWord()
	: StartTimestamp(0, 0, 0, 0),
	  EndTimestamp(0, 0, 0, 0),
	  Content(""),
	  bHasEndTimestamp(false),
	  Role(EDreamMusicLyricTextRole::Lyric)
{
}

FDreamMusicLyricWord::FDreamMusicLyricWord(FDreamMusicLyricTimestamp InStartTimestamp, FDreamMusicLyricTimestamp InEndTimestamp, FString InContent)
	: StartTimestamp(InStartTimestamp),
	  EndTimestamp(InEndTimestamp),
	  Content(InContent),
	  bHasEndTimestamp(false),
	  Role(EDreamMusicLyricTextRole::Lyric)
{
	if (InEndTimestamp.ToMilliseconds() > 0)
	{
		bHasEndTimestamp = true;
	}
}

FDreamMusicLyricWord::FDreamMusicLyricWord(const FString& InContent, const FDreamMusicLyricTimestamp& InStartTimestamp, const FDreamMusicLyricTimestamp& InEndTimestamp)
	: StartTimestamp(InStartTimestamp),
	  EndTimestamp(InEndTimestamp),
	  Content(InContent),
	  bHasEndTimestamp(false),
	  Role(EDreamMusicLyricTextRole::Lyric)
{
	if (InEndTimestamp.ToMilliseconds() > 0)
	{
		bHasEndTimestamp = true;
	}
}

bool FDreamMusicLyric::operator==(const FDreamMusicLyric& Target) const
{
	return Content == Target.Content && StartTimestamp == Target.StartTimestamp && Translate == Target.Translate;
}

bool FDreamMusicLyric::operator==(const FDreamMusicLyricTimestamp& Target) const
{
	return StartTimestamp == Target;
}

bool FDreamMusicLyric::operator!=(const FDreamMusicLyric& Target) const
{
	return !(*this == Target);
}

bool FDreamMusicInformation::IsValid() const
{
	return !Title.IsEmpty() || !Artist.IsEmpty() || !Album.IsEmpty() || Cover.IsValid() || !Genre.IsEmpty();
}

bool FDreamMusicInformation::operator==(const FDreamMusicInformation& Target) const
{
	return Title == Target.Title && Artist == Target.Artist && Album == Target.Album && Genre == Target.Genre && Cover == Target.Cover;
}

bool FDreamMusicInformationData::IsValid() const
{
	return Music.LoadSynchronous() != nullptr;
}

bool FDreamMusicInformationData::operator==(const FDreamMusicInformationData& Target) const
{
	return Music == Target.Music;
}

bool FDreamMusicDataStruct::IsValid() const
{
	return Information.IsValid() && Data.IsValid();
}

bool FDreamMusicDataStruct::operator==(const FDreamMusicDataStruct& Target) const
{
	return Information == Target.Information && Data == Target.Data;
}

bool FDreamMusicDataStruct::HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const
{
	for (UDreamMusicPlayerExpansionData* ExpansionData : ExpansionDatas)
	{
		if (ExpansionData == nullptr)
		{
			continue;
		}

		if (ExpansionData->GetClass() == ExpansionDataClass)
		{
			return true;
		}
	}

	return false;
}

// ============ 歌词资产相关类型实现 ============

int64 FDreamMusicLyricWord::GetDurationMilliseconds() const
{
	if (!bHasEndTimestamp)
	{
		return 0;
	}
	return static_cast<int64>(EndTimestamp.ToMilliseconds()) - static_cast<int64>(StartTimestamp.ToMilliseconds());
}

bool FDreamMusicLyricWord::IsTimeInRange(const FDreamMusicLyricTimestamp& Time) const
{
	if (Time < StartTimestamp)
	{
		return false;
	}
	if (!bHasEndTimestamp)
	{
		return true; // 如果没有结束时间，只要时间 >= 开始时间就认为在范围内
	}
	return Time <= EndTimestamp;
}

bool FDreamMusicLyricWord::IsEmpty() const
{
	return Content.IsEmpty();
}

FDreamMusicLyricTimestamp FDreamMusicLyricLine::GetStartTimestamp() const
{
	if (Words.Num() == 0)
	{
		return FDreamMusicLyricTimestamp();
	}
	return Words[0].StartTimestamp;
}

FDreamMusicLyricTimestamp FDreamMusicLyricLine::GetEndTimestamp() const
{
	if (Words.Num() == 0)
	{
		return FDreamMusicLyricTimestamp();
	}
	const FDreamMusicLyricWord& LastWord = Words[Words.Num() - 1];
	if (LastWord.bHasEndTimestamp)
	{
		return LastWord.EndTimestamp;
	}
	return LastWord.StartTimestamp;
}

bool FDreamMusicLyricLine::IsTimeInRange(const FDreamMusicLyricTimestamp& Time) const
{
	if (Words.Num() == 0)
	{
		return false;
	}
	FDreamMusicLyricTimestamp Start = GetStartTimestamp();
	FDreamMusicLyricTimestamp End = GetEndTimestamp();
	return Time >= Start && Time <= End;
}

bool FDreamMusicLyricLine::IsEmpty() const
{
	return Text.IsEmpty() && Words.Num() == 0;
}

FDreamMusicLyricLine* FDreamMusicLyricGroup::GetLineByRole(EDreamMusicLyricTextRole Role)
{
	for (FDreamMusicLyricLine& Line : Lines)
	{
		if (Line.Role == Role)
		{
			return &Line;
		}
	}
	return nullptr;
}

const FDreamMusicLyricLine* FDreamMusicLyricGroup::GetLineByRole(EDreamMusicLyricTextRole Role) const
{
	for (const FDreamMusicLyricLine& Line : Lines)
	{
		if (Line.Role == Role)
		{
			return &Line;
		}
	}
	return nullptr;
}

FDreamMusicLyricLine* FDreamMusicLyricGroup::GetMainLyricLine()
{
	FDreamMusicLyricLine* LyricLine = GetLineByRole(EDreamMusicLyricTextRole::Lyric);
	if (LyricLine)
	{
		return LyricLine;
	}
	// 如果没有找到 Lyric 角色，返回第一行
	if (Lines.Num() > 0)
	{
		return &Lines[0];
	}
	return nullptr;
}

bool FDreamMusicLyricGroup::IsTimeMatch(const FDreamMusicLyricTimestamp& Time, float ToleranceSeconds) const
{
	int32 ToleranceMs = static_cast<int32>(ToleranceSeconds * 1000.0f);
	int32 TimeMs = Time.ToMilliseconds();
	int32 TimestampMs = Timestamp.ToMilliseconds();
	int32 Diff = FMath::Abs(TimeMs - TimestampMs);
	return Diff <= ToleranceMs;
}

bool FDreamMusicLyricGroup::IsEmpty() const
{
	return Lines.Num() == 0 || Lines[0].IsEmpty();
}

FString FDreamMusicLyricMetadata::GetValue(const FString& Key, const FString& DefaultValue) const
{
	const FString* Value = Items.Find(Key);
	return Value ? *Value : DefaultValue;
}

void FDreamMusicLyricMetadata::SetValue(const FString& Key, const FString& Value)
{
	Items.Add(Key, Value);
}

bool FDreamMusicLyricMetadata::HasKey(const FString& Key) const
{
	return Items.Contains(Key);
}
