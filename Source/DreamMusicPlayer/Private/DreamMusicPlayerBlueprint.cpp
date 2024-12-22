// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerBlueprint.h"

#include "Classes/DreamMusicPlayerLyricTools.h"

TArray<FString> UDreamMusicPlayerBlueprint::GetLyricFileNames()
{
	return FDreamMusicPlayerLyricTools::GetLyricFileNames();
}
