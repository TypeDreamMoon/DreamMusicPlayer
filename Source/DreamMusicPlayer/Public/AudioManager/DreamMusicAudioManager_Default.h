// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicAudioManager.h"
#include "DreamMusicAudioManager_Default.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Default")
class DREAMMUSICPLAYER_API UDreamMusicAudioManager_Default : public UDreamMusicAudioManager
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	UAudioComponent* AudioComponent;
	
public:
	virtual UAudioComponent* GetAudioComponent() override;
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent) override;
	virtual bool IsPlaying() const override;
	virtual void Music_Changed(const FDreamMusicDataStruct& InMusicData) override;
	virtual void Music_Play(float InTime = 0.f) override;
	virtual void Music_Stop() override;
	virtual void Music_Pause() override;
	virtual void Music_UnPause() override;
	virtual void Music_Start() override;
};
