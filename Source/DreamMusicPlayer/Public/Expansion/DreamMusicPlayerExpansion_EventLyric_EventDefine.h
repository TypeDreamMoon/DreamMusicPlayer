// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DreamMusicPlayerExpansion_EventLyric_EventDefine.generated.h"

struct FDreamMusicLyric;

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DisplayName = "Event Lyric Event Define")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_EventLyric_EventDefine : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetPayload(UObject* InPayloadObject);

	UFUNCTION(BlueprintPure)
	UObject* GetPayload() const;

	UPROPERTY(BlueprintReadOnly)
	UObject* Payload;

	void CallEvent(const FString& EventName, const FDreamMusicLyric& Lyric);

public:
	/** UObject override **/
	virtual class UWorld* GetWorld() const override;
};
