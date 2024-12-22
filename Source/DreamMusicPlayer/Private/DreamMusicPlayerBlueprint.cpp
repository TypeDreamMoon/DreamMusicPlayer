// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerBlueprint.h"

#include "Classes/DreamMusicPlayerLyricTools.h"

TArray<FString> UDreamMusicPlayerBlueprint::GetLyricFileNames()
{
	return FDreamMusicPlayerLyricTools::GetLyricFileNames();
}

float UDreamMusicPlayerBlueprint::ConvLyricTimestampToFloat(FDreamMusicLyricTimestamp InTimestamp)
{
	return FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(InTimestamp);
}

FDreamMusicLyricTimestamp UDreamMusicPlayerBlueprint::ConvFloatToLyricTimestamp(float InFloat)
{
	return FDreamMusicPlayerLyricTools::Conv_TimestampFromFloat(InFloat);
}
