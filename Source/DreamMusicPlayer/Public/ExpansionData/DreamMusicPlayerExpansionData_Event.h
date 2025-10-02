// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_Event.generated.h"

class UDreamMusicPlayerPayload;

typedef const TPair<FString, UDreamMusicPlayerPayload*>& FDreamEventDefine;
typedef TFunctionRef<void(FDreamEventDefine)> FDreamEventCallback;

USTRUCT(BlueprintType)
struct FDreamMusicPlayerExpansionData_BaseEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TMap<FString, UDreamMusicPlayerPayload*> Events;

	void Call(FDreamEventCallback Callback) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct FDreamMusicPlayerExpansionData_Event_EventDefine
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicPlayerExpansionData_BaseEvent Event;

	bool operator==(int Other) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct FDreamMusicPlayerExpansionData_Event_TimeEventDefine
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicPlayerExpansionData_BaseEvent Event;

	bool operator==(const FDreamMusicLyricTimestamp& Other) const;
};

/**
 * 
 */
UCLASS(DisplayName = "Event Expansion Data")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansionData_Event : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_Event_EventDefine> MusicStartEventDefines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_Event_EventDefine> MusicEndEventDefines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_Event_EventDefine> LyricEventDefines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_Event_TimeEventDefine> TimeEventDefines;
};
