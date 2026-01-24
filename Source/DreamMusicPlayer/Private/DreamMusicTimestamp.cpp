#include "DreamMusicTimestamp.h"

#include "Kismet/KismetStringLibrary.h"


FDreamMusicTimestamp::FDreamMusicTimestamp(std::chrono::duration<double> Duration)
{
	FromSeconds(Duration.count());
}

FDreamMusicTimestamp::FDreamMusicTimestamp(float InSeconds)
{
	FromSeconds(InSeconds);
}

bool FDreamMusicTimestamp::operator==(const FDreamMusicTimestamp& Target) const
{
	return Target.ToMilliseconds() == ToMilliseconds();
}

bool FDreamMusicTimestamp::operator>=(const FDreamMusicTimestamp& Target) const
{
	return ToMilliseconds() >= Target.ToMilliseconds();
}

bool FDreamMusicTimestamp::operator>(const FDreamMusicTimestamp& Target) const
{
	return ToMilliseconds() > Target.ToMilliseconds();
}

bool FDreamMusicTimestamp::operator<=(const FDreamMusicTimestamp& Target) const
{
	return ToMilliseconds() <= Target.ToMilliseconds();
}

bool FDreamMusicTimestamp::operator<(const FDreamMusicTimestamp& Target) const
{
	return ToMilliseconds() < Target.ToMilliseconds();
}

const std::strong_ordering FDreamMusicTimestamp::operator<=>(const FDreamMusicTimestamp& Target) const
{
	// 首先比较小时
	if (auto Result = Hours <=> Target.Hours; Result != 0)
		return Result;

	// 然后比较分钟
	if (auto Result = Minute <=> Target.Minute; Result != 0)
		return Result;

	// 再比较秒
	if (auto Result = Seconds <=> Target.Seconds; Result != 0)
		return Result;

	// 最后比较毫秒
	return Millisecond <=> Target.Millisecond;
}

bool FDreamMusicTimestamp::IsApproximatelyEqual(const FDreamMusicTimestamp& Target, int ToleranceMilliseconds) const
{
	int Diff = ToMilliseconds() - Target.ToMilliseconds();
	return FMath::Abs(Diff) <= ToleranceMilliseconds;
}

const FDreamMusicTimestamp* FDreamMusicTimestamp::FromSeconds(float InSeconds)
{
	Seconds = FMath::FloorToInt(InSeconds);
	Millisecond = FMath::RoundToInt((InSeconds - Seconds) * 1000);
	Hours = Seconds / 3600;
	Seconds %= 3600;
	Minute = Seconds / 60;
	Seconds = Seconds % 60;
	return this;
}

int FDreamMusicTimestamp::ToMilliseconds() const
{
	return Hours * 3600000 + Minute * 60000 + Seconds * 1000 + Millisecond;
}

float FDreamMusicTimestamp::ToSeconds() const
{
	float TotalSeconds = Hours * 3600.0f;
	TotalSeconds += Minute * 60.0f;
	TotalSeconds += Seconds;
	TotalSeconds += Millisecond / 1000.0f;

	return TotalSeconds;
}

int64 FDreamMusicTimestamp::ToTotalMilliseconds() const
{
	return static_cast<int64>(Hours) * 3600000LL +
		static_cast<int64>(Minute) * 60000LL +
		static_cast<int64>(Seconds) * 1000LL +
		static_cast<int64>(Millisecond);
}

FDreamMusicTimestamp FDreamMusicTimestamp::FromTotalMilliseconds(int64 TotalMilliseconds)
{
	FDreamMusicTimestamp Result;
	Result.Hours = static_cast<int32>(TotalMilliseconds / 3600000LL);
	TotalMilliseconds %= 3600000LL;
	Result.Minute = static_cast<int32>(TotalMilliseconds / 60000LL);
	TotalMilliseconds %= 60000LL;
	Result.Seconds = static_cast<int32>(TotalMilliseconds / 1000LL);
	Result.Millisecond = static_cast<int32>(TotalMilliseconds % 1000LL);
	return Result;
}

FDreamMusicTimestamp FDreamMusicTimestamp::FromSecondsStatic(float TotalSeconds)
{
	return FromTotalMilliseconds(static_cast<int64>(TotalSeconds * 1000.0f));
}

void FDreamMusicTimestamp::Normalize()
{
	int64 TotalMs = ToTotalMilliseconds();
	if (TotalMs < 0)
	{
		TotalMs = 0;
	}

	*this = FromTotalMilliseconds(TotalMs);
}

FString FDreamMusicTimestamp::ToStringFormatted(bool bIncludeHours, int32 FractionalDigits) const
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

bool FDreamMusicTimestamp::IsZero() const
{
	return Hours == 0 && Minute == 0 && Seconds == 0 && Millisecond == 0;
}

FDreamMusicTimestamp FDreamMusicTimestamp::Parse(const FString& TimestampStr)
{
	FDreamMusicTimestamp Ts;

	FString TimeString = TimestampStr;

	// Try HH:MM:SS,mmm or HH:MM:SS.mmm (SRT/ASS Format)
	{
		FRegexPattern RegexPattern(TEXT("((\\d{1,2}):(\\d{2}):(\\d{2})[,.](\\d{3}))"));
		FRegexMatcher RegexMatcher(RegexPattern, TimeString);

		if (RegexMatcher.FindNext())
		{
			Ts.Hours = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(0));
			Ts.Minute = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(1));
			Ts.Seconds = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(2));
			Ts.Millisecond = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(3));
			return Ts;
		}
	}

	// Try MM:SS.mmm (LRC Format)
	{
		FRegexPattern RegexPattern(TEXT("((\\d{1,2}):(\\d{2}):(\\d{2})[.](\\d{3}))"));
		FRegexMatcher RegexMatcher(RegexPattern, TimeString);
		if (RegexMatcher.FindNext())
		{
			Ts.Minute = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(0));
			Ts.Seconds = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(1));
			Ts.Millisecond = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(2));
			return Ts;
		}
	}

	// Try MM:SS.mm (LRC Format with 2-digit milliseconds)
	{
		FRegexPattern RegexPattern(TEXT("((\\d{1,2}):(\\d{2})\\.(\\d{2}))"));
		FRegexMatcher RegexMatcher(RegexPattern, TimeString);

		if (RegexMatcher.FindNext())
		{
			Ts.Minute = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(0));
			Ts.Seconds = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(1));
			Ts.Millisecond = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(2)) * 10; // Convert to 3-digit milliseconds
		}
	}

	return Ts;
}

FDreamMusicTimestamp FDreamMusicTimestamp::operator+(const FDreamMusicTimestamp& Other) const
{
	return FromTotalMilliseconds(ToTotalMilliseconds() + Other.ToTotalMilliseconds());
}

FDreamMusicTimestamp FDreamMusicTimestamp::operator-(const FDreamMusicTimestamp& Other) const
{
	int64 Result = ToTotalMilliseconds() - Other.ToTotalMilliseconds();
	return FromTotalMilliseconds(FMath::Max<int64>(0, Result));
}
