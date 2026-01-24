// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamMusicPlayerLyricBlueprint.generated.h"

/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYERLYRIC_API UDreamMusicPlayerLyricBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Lyric")
	static TArray<FString> GetLyricFileNames();
};
