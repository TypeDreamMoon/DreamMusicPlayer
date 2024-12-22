// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamMusicPlayerBlueprint.generated.h"

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
};
