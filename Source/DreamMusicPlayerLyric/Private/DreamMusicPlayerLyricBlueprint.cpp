// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerLyricBlueprint.h"

#include "DreamLyricUtils.h"

TArray<FString> UDreamMusicPlayerLyricBlueprint::GetLyricFileNames()
{
	return FDreamLyricUtils::GetLyricFileNames();
}
