#include "DreamMusicPlayerCommon.h"

#include "ConstantQNRT.h"
#include "DreamMusicPlayerLog.h"
#include "LoudnessNRT.h"
#include "Engine/ObjectLibrary.h"

bool FDreamMusicLyricTimestamp::operator==(const FDreamMusicLyricTimestamp& Target) const
{
	return Millisecond == Target.Millisecond && Minute == Target.Minute && Seconds == Target.Seconds;
}

bool FDreamMusicLyricTimestamp::operator>=(const FDreamMusicLyricTimestamp& Target) const
{
	if (Minute >= Target.Minute)
		return true;
	else if (Seconds >= Target.Seconds)
		return true;
	else if (Millisecond >= Target.Millisecond)
		return true;
	return false;
}

bool FDreamMusicLyricTimestamp::operator>(const FDreamMusicLyricTimestamp& Target) const
{
	if (Minute > Target.Minute)
		return true;
	else if (Seconds > Target.Seconds)
		return true;
	else if (Millisecond > Target.Millisecond)
		return true;
	else
		return false;
}

bool FDreamMusicLyricTimestamp::operator<=(const FDreamMusicLyricTimestamp& Target) const
{
	if (Minute <= Target.Minute)
		return true;
	else if (Seconds <= Target.Seconds)
		return true;
	else if (Millisecond <= Target.Millisecond)
		return true;
	else
		return false;
}

bool FDreamMusicLyricTimestamp::operator<(const FDreamMusicLyricTimestamp& Target) const
{
	if (Minute < Target.Minute)
		return true;
	else if (Seconds < Target.Seconds)
		return true;
	else if (Millisecond < Target.Millisecond)
		return true;
	else
		return false;
}

FDreamMusicLyric::FDreamMusicLyric(FString Line)
{
	FString CleamTime = Line.Mid(1, 8);
	FString M = CleamTime.Left(2);
	FString S = CleamTime.Mid(3, 2);
	FString MS = CleamTime.Right(2);
	FDreamMusicLyricTimestamp Time = FDreamMusicLyricTimestamp(
		FCString::Atoi(*M), FCString::Atoi(*S), FCString::Atoi(*MS));
	Content = Line.Mid(Line.Find("]") + 1);
	Timestamp = Time;
	Translate = "";
	return;
}

bool FDreamMusicLyric::operator==(const FDreamMusicLyric& Target) const
{
	return Content == Target.Content && Timestamp == Target.Timestamp && Translate == Target.Translate;
}

bool FDreamMusicLyric::operator==(const FDreamMusicLyricTimestamp& Target) const
{
	return Timestamp == Target;
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
	return Music == Target.Music && ConstantQ == Target.ConstantQ && Loudness == Target.Loudness;
}

bool FDreamMusicDataStruct::IsVaild() const
{
	return Information.IsValid() && Data.IsValid();
}

bool FDreamMusicDataStruct::operator==(const FDreamMusicDataStruct& Target) const
{
	return Information == Target.Information && Data == Target.Data;
}
