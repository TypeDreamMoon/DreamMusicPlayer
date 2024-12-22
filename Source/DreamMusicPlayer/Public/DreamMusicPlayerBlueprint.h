// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamMusicPlayerBlueprint.generated.h"

struct FDreamMusicLyricTimestamp;
/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicPlayerBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure)
	static TArray<FString> GetLyricFileNames();

	UFUNCTION(BlueprintPure)
	static float ConvLyricTimestampToFloat(FDreamMusicLyricTimestamp InTimestamp);

	UFUNCTION(BlueprintPure)
	static FDreamMusicLyricTimestamp ConvFloatToLyricTimestamp(float InFloat);
};
