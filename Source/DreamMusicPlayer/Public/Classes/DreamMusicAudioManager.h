// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DreamMusicAudioManager.generated.h"

struct FDreamMusicLyricTimestamp;
struct FDreamMusicDataStruct;
class UDreamMusicPlayerComponent;
/**
 * 播放中间件
 */
UCLASS(EditInlineNew, Abstract, Blueprintable, BlueprintType)
class DREAMMUSICPLAYER_API UDreamMusicAudioManager : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	AActor* Owner;

	UPROPERTY(BlueprintReadOnly)
	UDreamMusicPlayerComponent* MusicPlayerComponent;

public:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent);
	virtual void Deinitialize();
	virtual bool IsPlaying() const;
	virtual void Tick(const FDreamMusicLyricTimestamp& InTimestamp, float DeltaTime);
	virtual void Music_Changed(const FDreamMusicDataStruct& InMusicData);
	virtual void Music_Play(float InTime = 0.f);
	virtual void Music_Start();
	virtual void Music_Stop();
	virtual void Music_Pause();
	virtual void Music_UnPause();
	virtual void Music_End();
	virtual UAudioComponent* GetAudioComponent();

	/**
	 * Check if audio component is ready for use
	 */
	bool IsAudioComponentReady(UAudioComponent* Component) const;

public:
	UFUNCTION(BlueprintPure)
	inline AActor* GetOwner() const { return Owner; }
};
