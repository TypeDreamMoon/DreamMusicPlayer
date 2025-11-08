// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_Event.generated.h"

struct FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine;
class UDreamMusicPlayerPayload;


typedef TFunctionRef<void(const FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine&)> FDreamEventCallback;

USTRUCT(BlueprintType)
struct FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	UDreamMusicPlayerPayload* Payload;
};

USTRUCT(BlueprintType)
struct FDreamMusicPlayerExpansionData_BaseEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine> Events;

	void Call(FDreamEventCallback Callback) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct FDreamMusicPlayerExpansionData_Event_LyricEventDefine
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
class DREAMMUSICPLAYERLYRIC_API UDreamMusicPlayerExpansionData_Event : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_BaseEvent> MusicStartEventDefines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_BaseEvent> MusicEndEventDefines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_Event_LyricEventDefine> LyricEventDefines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TArray<FDreamMusicPlayerExpansionData_Event_TimeEventDefine> TimeEventDefines;
};
