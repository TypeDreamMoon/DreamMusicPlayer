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
