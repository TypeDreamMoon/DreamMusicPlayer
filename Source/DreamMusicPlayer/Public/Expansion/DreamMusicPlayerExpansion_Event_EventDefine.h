// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_Event.h"
#include "UObject/Object.h"
#include "DreamMusicPlayerExpansion_Event_EventDefine.generated.h"

class UDreamMusicPlayerPayload;
struct FDreamMusicLyric;

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DisplayName = "Event Define")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_Event_EventDefine : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetPayload(UObject* InPayloadObject);

	UFUNCTION(BlueprintPure)
	UObject* GetPayload() const;

	UPROPERTY(BlueprintReadOnly)
	UObject* Payload;

	void CallEvent(const FString& EventName, const FDreamMusicLyric& Lyric, UDreamMusicPlayerPayload* InEventPayload);
	void CallEvent(FDreamEventDefine Event);

public:
	/** UObject override **/
	virtual class UWorld* GetWorld() const override;
};
