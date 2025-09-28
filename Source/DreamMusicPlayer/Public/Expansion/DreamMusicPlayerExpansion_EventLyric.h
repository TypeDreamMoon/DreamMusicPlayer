// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_EventLyric.generated.h"

class UDreamMusicPlayerExpansion_EventLyric_EventDefine;

/**
 * 
 */
UCLASS(DisplayName = "Event Lyric")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_EventLyric : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Event Lyric")
	UDreamMusicPlayerExpansion_EventLyric_EventDefine* EventDefineObject;

public:
	UFUNCTION(BlueprintCallable)
	void SetPayload(UObject* InPayloadObject);

protected:
	TWeakObjectPtr<UObject> Payload;
	
protected:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent) override;

	void OnLyricChangedHandle(FDreamMusicLyric Lyric, int Index);
};
