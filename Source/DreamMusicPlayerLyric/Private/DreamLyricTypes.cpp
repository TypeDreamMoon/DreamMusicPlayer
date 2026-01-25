#include "DreamLyricTypes.h"

#include "ExpansionData/DreamMusicPlayerExpansionData_Lyric.h"


FDreamMusicLyricWord::FDreamMusicLyricWord()
	: StartTimestamp(0, 0, 0, 0),
	  EndTimestamp(0, 0, 0, 0),
	  Content("")

{
}

FDreamMusicLyricWord::FDreamMusicLyricWord(FDreamMusicTimestamp InStartTimestamp, FDreamMusicTimestamp InEndTimestamp, FString InContent)
	: StartTimestamp(InStartTimestamp),
	  EndTimestamp(InEndTimestamp),
	  Content(InContent)
{
}

FDreamMusicLyricWord::FDreamMusicLyricWord(const FString& InContent, const FDreamMusicTimestamp& InStartTimestamp, const FDreamMusicTimestamp& InEndTimestamp)
	: StartTimestamp(InStartTimestamp),
	  EndTimestamp(InEndTimestamp),
	  Content(InContent)
{
}

bool FDreamMusicLyric::operator==(const FDreamMusicLyric& Target) const
{
	return Content == Target.Content && StartTimestamp == Target.StartTimestamp && EndTimestamp == Target.EndTimestamp;
}

bool FDreamMusicLyric::operator==(const FDreamMusicTimestamp& Target) const
{
	return StartTimestamp == Target;
}

bool FDreamMusicLyric::operator!=(const FDreamMusicLyric& Target) const
{
	return !(*this == Target);
}

int64 FDreamMusicLyricWord::GetDurationMilliseconds() const
{
	return static_cast<int64>(EndTimestamp.ToMilliseconds()) - static_cast<int64>(StartTimestamp.ToMilliseconds());
}

bool FDreamMusicLyricWord::IsTimeInRange(const FDreamMusicTimestamp& Time) const
{
	if (Time < StartTimestamp)
	{
		return false;
	}
	return Time <= EndTimestamp;
}

bool FDreamMusicLyricWord::IsEmpty() const
{
	return Content.IsEmpty();
}

bool FDreamMusicLyricWord::operator==(const FDreamMusicLyricWord& Target) const
{
	if (Content != Target.Content)
	{
		return false;
	}

	if (StartTimestamp != Target.StartTimestamp)
	{
		return false;
	}

	if (EndTimestamp != Target.EndTimestamp)
	{
		return false;
	}

	return true;
}

FString FDreamMusicLyricWord::ToString() const
{
	FStringBuilderBase StringBuilder;

	StringBuilder << "[Content]:" << Content << "\n";
	StringBuilder << "[StartTime]:" << StartTimestamp.ToString() << "\n";
	StringBuilder << "[EndTime]:" << EndTimestamp.ToString() << "\n";

	return StringBuilder.ToString();
}

FDreamMusicTimestamp FDreamMusicLyricLine::GetStartTimestamp() const
{
	if (Words.Num() == 0)
	{
		return FDreamMusicTimestamp();
	}
	return Words[0].StartTimestamp;
}

FDreamMusicTimestamp FDreamMusicLyricLine::GetEndTimestamp() const
{
	if (Words.Num() == 0)
	{
		return FDreamMusicTimestamp();
	}
	const FDreamMusicLyricWord& LastWord = Words[Words.Num() - 1];
	return LastWord.StartTimestamp;
}

bool FDreamMusicLyricLine::IsTimeInRange(const FDreamMusicTimestamp& Time) const
{
	if (Words.Num() == 0)
	{
		return false;
	}
	FDreamMusicTimestamp Start = GetStartTimestamp();
	FDreamMusicTimestamp End = GetEndTimestamp();
	return Time >= Start && Time <= End;
}

bool FDreamMusicLyricLine::IsEmpty() const
{
	return Text.IsEmpty() && Words.Num() == 0;
}

bool FDreamMusicLyricLine::operator==(const FDreamMusicLyricLine& Target) const
{
	if (Text != Target.Text)
	{
		return false;
	}
	if (Words != Target.Words)
	{
		return false;
	}
	if (Role != Target.Role)
	{
		return false;
	}
	return true;
}

FString FDreamMusicLyricLine::ToString() const
{
	FStringBuilderBase StringBuilder;

	StringBuilder << "[Text]:" << Text << "\n";
	StringBuilder << "[Words]:" << "\n";
	for (const FDreamMusicLyricWord& Word : Words)
	{
		StringBuilder << "  " << Word.ToString() << "\n";
	}
	StringBuilder << "[Role]:" << UEnum::GetDisplayValueAsText(Role).ToString() << "\n";

	return StringBuilder.ToString();
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

bool FDreamMusicLyricGroup::IsTimeMatch(const FDreamMusicTimestamp& Time, float ToleranceSeconds) const
{
	int32 ToleranceMs = static_cast<int32>(ToleranceSeconds * 1000.0f);
	int32 TimeMs = Time.ToMilliseconds();
	int32 TimestampMs = StartTimestamp.ToMilliseconds();
	int32 Diff = FMath::Abs(TimeMs - TimestampMs);
	return Diff <= ToleranceMs;
}

bool FDreamMusicLyricGroup::IsEmpty() const
{
	return Lines.Num() == 0 || Lines[0].IsEmpty();
}

bool FDreamMusicLyricGroup::IsRomanizationWordsEmpty() const
{
	return GetLineByRole(EDreamMusicLyricTextRole::Romanization)->Words.IsEmpty();
}

bool FDreamMusicLyricGroup::IsWordsEmpty() const
{
	return GetLineByRole(EDreamMusicLyricTextRole::Lyric)->Words.IsEmpty();
}

const FDreamMusicLyricLine* FDreamMusicLyricGroup::operator[](EDreamMusicLyricTextRole InRole) const
{
	return GetLineByRole(InRole);
}

bool FDreamMusicLyricGroup::operator==(const FDreamMusicLyricGroup& Other) const
{
	// 比较 StartTimestamp
	if (StartTimestamp != Other.StartTimestamp)
		return false;

	// 比较 EndTimestamp
	if (EndTimestamp != Other.EndTimestamp)
		return false;

	// 比较 Lines 数组
	if (Lines.Num() != Other.Lines.Num())
		return false;

	// 比较 Lines 中的每一行
	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		if (Lines[i] != Other.Lines[i]) // 需要确保 FDreamMusicLyricLine 也重载了 operator==
			return false;
	}

	// 如果所有比较都相等，返回 true
	return true;
}

bool FDreamMusicLyricGroup::operator==(const FDreamMusicLyricGroup* Other) const
{
	return *this == *Other;
}

FString FDreamMusicLyricGroup::ToString() const
{
	FStringBuilderBase StringBuilder;

	StringBuilder << "[StartTime]:" << StartTimestamp.ToString() << "\n";
	StringBuilder << "[EndTime]:" << EndTimestamp.ToString() << "\n";
	StringBuilder << "[Lines]:" << "\n";
	for (const FDreamMusicLyricLine& Line : Lines)
	{
		StringBuilder << "  " << Line.ToString() << "\n";
	}
	StringBuilder << "[Metadata]:" << "\n";

	return StringBuilder.ToString();
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

bool FDreamLyricParserOptionGroup::operator==(const FDreamLyricParserOptionGroup& Other) const
{
	for (const EDreamMusicLyricTextRole& Role : Roles)
	{
		if (!Other.Roles.Contains(Role))
		{
			return false;
		}
	}

	return true;
}

bool FDreamLyricParserOptions::IsEmpty() const
{
	return GroupingSequence.IsEmpty();
}

void FDreamLyricParserOptions::operator+=(EDreamMusicLyricTextRole Role)
{
	if (GroupingSequence.Contains(Role))
	{
		return;
	}
	else
	{
		GroupingSequence.AddUnique(Role);
	}
}

void FDreamLyricParserOptions::operator-=(EDreamMusicLyricTextRole Role)
{
	if (GroupingSequence.Contains(Role))
	{
		GroupingSequence.Remove(Role);
	}
}

bool FDreamLyricParserOptions::operator[](EDreamMusicLyricTextRole Role)
{
	return GroupingSequence.Contains(Role);
}
