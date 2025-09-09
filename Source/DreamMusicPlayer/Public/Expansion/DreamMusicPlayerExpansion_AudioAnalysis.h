// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_AudioAnalysis.generated.h"

UENUM(BlueprintType)
enum class EDreamMusicPlayerExpansion_AudioAnalysis_AverageType : uint8
{
	// Left Div Right
	Left_Right UMETA(DisplayName = "Left And Right"),
	// Right Div Left
	Right_Left UMETA(DisplayName = "Right And Left"),
	// Only Left
	Left UMETA(DisplayName = "Only Left"),
	// Only Right
	Right UMETA(DisplayName = "Only Right"),
};

/**
 * 
 */
UCLASS(DisplayName = "Audio Analysis")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_AudioAnalysis : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisTextureLoaded, UTexture2D*, Texture);

public:
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

	UPROPERTY(BlueprintReadOnly, Category = "State", Transient)
	TArray<float> ConstantQData;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataAverage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	EDreamMusicPlayerExpansion_AudioAnalysis_AverageType AverageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bEnableCreateAnalysisTexture = false;

public:
	/**
	 * Get Current Duration Music NRT Data
	 * @param ConstantNrtL Current Music Duration L Channel NRT Data
	 * @param ConstantNrtR Current Music Duration R Channel NRT Data
	 * @param ConstantNrtAverage NRT Data After Averaging L And R Channels
	 * @param OutLoudnessValue Current Music Duration Loudness Value
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	void GetAudioNrtData(TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR, TArray<float>& ConstantNrtAverage,
	                     float& OutLoudnessValue);

	UFUNCTION(BlueprintPure, Category = "Functions")
	UTexture2D* GetAnalysisTexture() { return Internal_AnalysisTexture; }

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisTextureLoaded OnAnalysisTextureLoaded;

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
	virtual void BP_MusicStart_Implementation() override;

	static uint8* BuildPixelArray(const TArray<float>& Data);
	static void WriteTextureFromPixel(uint8* PixelArray, UTexture2D* Texture2D);

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Internal_AnalysisTexture;

	bool bIsCreated = false;
};
