// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_AudioAnalysis.generated.h"

/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_AudioAnalysis : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	// Current Music Wave
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<USoundWave> SoundWave;

	// Current Music ConstantQ Data
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UConstantQNRT> ConstantQ;

	// Current Music Loudness Data
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<ULoudnessNRT> Loudness;

	// Current Music Duration L Channel ConstantQ NRT Data
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataL;

	// Current Music Duration R Channel ConstantQ NRT Data
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataR;

	// Current Music Duration Loudness NRT Data
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float LoudnessValue;

public:
	/**
	 * Get Current Duration Music NRT Data
	 * @param bConstantReverse Reverse Average?
	 * @param ConstantNrtL Current Music Duration L Channel NRT Data
	 * @param ConstantNrtR Current Music Duration R Channel NRT Data
	 * @param ConstantNrtAverage NRT Data After Averaging L And R Channels
	 * @param OutLoudnessValue Current Music Duration Loudness Value
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	void GetAudioNrtData(bool bConstantReverse, TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR, TArray<float>& ConstantNrtAverage,
						 float& OutLoudnessValue);
	
protected:
	/**
	 * Load NRT Data
	 */
	void LoadAudioNrt();

	/**
	 * 更新音频分析数据
	 */
	void UpdateAudioAnalysisData();

	virtual void BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData) override;
	virtual void BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime) override;
};
