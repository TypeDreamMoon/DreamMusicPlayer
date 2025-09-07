// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_AudioAnalysis.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Audio Analysis")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansionData_AudioAnalysis : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	// 频谱可视化对象
	UPROPERTY(Category="Visual", EditAnywhere, BlueprintReadWrite, meta=(MetaClass = "ConstantQNRT"))
	FSoftObjectPath ConstantQ;

	// 响度可视化对象
	UPROPERTY(Category="Visual", EditAnywhere, BlueprintReadWrite, meta=(MetaClass = "LoudnessNRT"))
	FSoftObjectPath Loudness;
};
