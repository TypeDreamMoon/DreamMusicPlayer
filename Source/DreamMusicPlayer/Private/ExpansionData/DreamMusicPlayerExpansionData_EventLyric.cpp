// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpansionData/DreamMusicPlayerExpansionData_EventLyric.h"

bool FDreamMusicPlayerExpansionData_EventLyric_EventDefine::operator==(int Other) const
{
	return Other == Index;
}

bool FDreamMusicPlayerExpansionData_EventLyric_TimeEventDefine::operator==(const FDreamMusicLyricTimestamp& Other) const
{
	return Other == Time;
}
