// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_MusicVideo.h"
#include "DreamMusicPlayerExpansion_MusicVideo.generated.h"

class UDreamMusicPlayerExpansionData_MusicVideo;
class UMediaPlayer;

/**
 * 
 */
UCLASS(DisplayName = "Music Video")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_MusicVideo : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	UMediaPlayer* MediaPlayer;

	UFUNCTION()
	void OnMediaOpenedHandle(FString OpenedUrl);

protected:
	virtual void BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData) override;

	UPROPERTY()
	FDreamMusicPlayerExpansionData_MusicVideo_Define CachedMusicVideoData;
};
