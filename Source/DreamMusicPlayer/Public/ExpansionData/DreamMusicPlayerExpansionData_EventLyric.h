// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_EventLyric.generated.h"

class UDreamMusicPlayerPayload;

USTRUCT(BlueprintType, Blueprintable)
struct FDreamMusicPlayerExpansionData_EventLyric_EventDefine
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TMap<FString, UDreamMusicPlayerPayload*> Events;

	bool operator==(int Other) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct FDreamMusicPlayerExpansionData_EventLyric_TimeEventDefine
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TMap<FString, UDreamMusicPlayerPayload*> Events;

	bool operator==(const FDreamMusicLyricTimestamp& Other) const;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Lyric")
	TArray<FDreamMusicPlayerExpansionData_EventLyric_TimeEventDefine> TimeEventDefines;
};
