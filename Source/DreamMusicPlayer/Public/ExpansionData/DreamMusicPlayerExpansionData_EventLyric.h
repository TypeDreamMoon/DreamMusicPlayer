// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_EventLyric.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FDreamMusicPlayerExpansionData_EventLyric_EventDefine
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventName;

	bool operator==(int Other) const;
};

/**
 * 
 */
UCLASS(DisplayName = "Event Lyric")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansionData_EventLyric : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Lyric")
	TArray<FDreamMusicPlayerExpansionData_EventLyric_EventDefine> EventDefines;
};
