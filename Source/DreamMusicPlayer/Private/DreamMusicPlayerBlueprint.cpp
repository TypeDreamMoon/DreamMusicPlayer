// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerBlueprint.h"

#include "LyricParser/DreamMusicPlayerLyricTools.h"

TArray<FString> UDreamMusicPlayerBlueprint::GetLyricFileNames()
{
	return FDreamMusicPlayerLyricTools::GetLyricFileNames();
}

float UDreamMusicPlayerBlueprint::ConvLyricTimestampToFloat(FDreamMusicLyricTimestamp InTimestamp)
{
	return InTimestamp.ToSeconds();
}

FDreamMusicLyricTimestamp UDreamMusicPlayerBlueprint::ConvFloatToLyricTimestamp(float InFloat)
{
	return *FDreamMusicLyricTimestamp().FromSeconds(InFloat);
}
