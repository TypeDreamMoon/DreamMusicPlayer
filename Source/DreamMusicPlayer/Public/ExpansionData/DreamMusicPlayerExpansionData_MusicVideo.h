// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_MusicVideo.generated.h"

class UBaseMediaSource;
class UFileMediaSource;
class UStreamMediaSource;
class UImgMediaSource;

USTRUCT(BlueprintType)
struct FDreamMusicPlayerExpansionData_MusicVideo_Define
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Can Loop")
	bool bCanLoop = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Media Source", meta=(EditConditionHides))
	UBaseMediaSource* MediaSource;

	UBaseMediaSource* GetMediaSource() const;
};

/**
 * 
 */
UCLASS(DisplayName = "Music Video Expansion Data", Blueprintable, BlueprintType)
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansionData_MusicVideo : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Video")
	FDreamMusicPlayerExpansionData_MusicVideo_Define MusicVideo;

	UFUNCTION(BlueprintPure)
	UBaseMediaSource* GetMediaSource() const;
};
