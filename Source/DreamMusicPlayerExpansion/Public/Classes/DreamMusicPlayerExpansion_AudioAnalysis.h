// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_AudioAnalysis.generated.h"

struct FLKFSResults;
class ULoudnessSettings;
class ULKFSSettings;
class UConstantQSettings;
struct FLKFSNRTResults;
struct FLoudnessResults;
struct FConstantQResults;

struct FSpectrumRingBuffer
{
	// 二维数组：[HistoryIndex][BandIndex]
	TArray<TArray<float>> Buffer;
    
	// 当前写入位置的指针（头部）
	int32 WriteIndex = 0;
    
	// 缓冲区最大容量（帧数）
	int32 MaxHistorySize = 0;

	// 初始化/重置大小
	void Resize(int32 InHistorySize, int32 InNumBands)
	{
		MaxHistorySize = InHistorySize;
		WriteIndex = 0;
		Buffer.Empty();
		Buffer.SetNum(MaxHistorySize);
        
		// 预分配内存
		for (auto& Frame : Buffer)
		{
			Frame.SetNumZeroed(InNumBands);
		}
	}

	// 写入新的一帧数据
	void PushFrame(const TArray<float>& NewData)
	{
		if (MaxHistorySize <= 0 || Buffer.IsEmpty()) return;

		// 写入当前位置（覆盖最旧的数据）
		if (NewData.Num() == Buffer[WriteIndex].Num())
		{
			Buffer[WriteIndex] = NewData;
		}
		else
		{
			Buffer[WriteIndex] = NewData; 
		}

		// 移动指针，环形回绕
		WriteIndex = (WriteIndex + 1) % MaxHistorySize;
	}

	// 获取“过去 N 帧”的平均能量
	float GetAverageEnergyOfRecentFrames(int32 FramesToLookBack, int32 LowBandIndex, int32 HighBandIndex) const
	{
		if (MaxHistorySize == 0) return 0.0f;

		float TotalEnergy = 0.0f;
		int32 Count = 0;
		int32 ActualFrames = FMath::Min(FramesToLookBack, MaxHistorySize);

		for (int32 i = 0; i < ActualFrames; ++i)
		{
			int32 ReadIdx = (WriteIndex - 1 - i + MaxHistorySize) % MaxHistorySize;
			const TArray<float>& FrameData = Buffer[ReadIdx];
            
			for (int32 Band = LowBandIndex; Band <= HighBandIndex; ++Band)
			{
				if (FrameData.IsValidIndex(Band))
				{
					TotalEnergy += FrameData[Band];
					Count++;
				}
			}
		}
		return (Count > 0) ? (TotalEnergy / Count) : 0.0f;
	}
};

namespace Audio
{
	class FConstantQResult;
}

class UConstantQAnalyzer;
class ULKFSAnalyzer;
class ULoudnessAnalyzer;

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
class DREAMMUSICPLAYEREXPANSION_API UDreamMusicPlayerExpansion_AudioAnalysis : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisTextureLoaded, const TArray<UTexture2D*>&, Texture);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisConstantQResult, const TArray<FConstantQResults>&, Results);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisLoudnessResult, const FLoudnessResults&, Results);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisLKFSResult, const FLKFSResults&, Results);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatDetected, float, BeatIntensity);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoStartAudioBus = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	UAudioBus* AnalysisAudioBus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableConstantQAnalysis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableLoudnessAnalysis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableLKFSAnalysis = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bEnableCreateAnalysisTexture = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EDreamMusicPlayerExpansion_AudioAnalysis_AverageType AverageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Instanced, Meta = (EditCondition = "bEnableConstantQAnalysis", EditConditionHides))
	UConstantQSettings* ConstantQSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Instanced, Meta = (EditCondition = "bEnableLoudnessAnalysis", EditConditionHides))
	ULoudnessSettings* LoudnessSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Instanced, Meta = (EditCondition = "bEnableLKFSAnalysis", EditConditionHides))
	ULKFSSettings* LKFSSettings;
	
	// 敏感度：当前能量必须是平均值的多少倍才触发？(推荐 1.3 - 1.5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Analysis|Beat Settings")
	float BeatSensitivity = 1.4f;

	// 冷却时间：两次鼓点之间的最小间隔 (秒)，防止连击 (推荐 0.15s - 0.25s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Analysis|Beat Settings")
	float BeatCooldownDuration = 0.2f;

	// 低频截止比例：我们只分析 CQT 频谱的前百分之多少？(底鼓通常在最左边，0.2 表示前 20%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Analysis|Beat Settings")
	float LowFreqBandRatio = 0.2f;
	
	// 历史缓冲区大小：保存多少帧？(假设 60fps，60帧就是1秒历史)
	UPROPERTY(EditAnywhere, Category = "Audio Analysis|Buffer")
	int32 HistoryBufferSize = 60;

public:
	UFUNCTION(BlueprintPure, Category = "Functions")
	TArray<UTexture2D*> GetAnalysisTexture() { return Internal_AnalysisTextures; }

	UFUNCTION(BlueprintPure, Category = "Functions")
	UTexture2D* GetAnalysisTextureAtChannel(int32 Channel) { return Internal_AnalysisTextures.IsValidIndex(Channel) ? Internal_AnalysisTextures[Channel] : nullptr; }

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisTextureLoaded OnAnalysisTextureLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisConstantQResult OnAnalysisConstantQResult;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisLoudnessResult OnAnalysisLoudnessResult;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisLKFSResult OnAnalysisLKFSResult;
	
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis|Beat")
	FOnBeatDetected OnKickDetected;

protected:
	virtual void BP_Initialize_Implementation(UDreamMusicPlayerComponent* InComponent) override;
	virtual void BP_MusicStart_Implementation() override;
	virtual void BP_ChangeMusic_Implementation(const FDreamMusicData& InData) override;
	
	void UpdateConstantQAnalysisData();
	void UpdateTextureFromSpectrum(UTexture2D* InTexture, const TArray<float>& InSpectrumData);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> Internal_AnalysisTextures;

	UPROPERTY(Transient)
	UConstantQAnalyzer* Internal_ConstantQAnalyzer;

	UPROPERTY(Transient)
	ULoudnessAnalyzer* Internal_LoudnessAnalyzer;

	UPROPERTY(Transient)
	ULKFSAnalyzer* Internal_LKFSAnalyzer;

	bool bIsCreated = false;

	int32 Internal_ChannelNums;
	TArray<FConstantQResults> Internal_ConstantQResultBuffer;
	FCriticalSection DataGuard;
	TArray<FSpectrumRingBuffer> Internal_ChannelHistoryBuffers;
	
	float RunningAverageEnergy = 0.0f; // 动态平均能量
	double LastBeatTriggerTime = 0.0;  // 上次触发的时间
	bool bIsBufferInitialized = false;

	UFUNCTION()
	void OnAnalysisConstantQResults(UConstantQAnalyzer* Analyzer, int32 ChannelIndex, const FConstantQResults& Results);
	UFUNCTION()
	void OnAnalysisLoudnessResults(const FLoudnessResults& Results);
	UFUNCTION()
	void OnAnalysisLKFSResults(ULKFSAnalyzer* Analyzer, const FLKFSResults& Results);
	
	// 辅助函数：执行检测
	void ProcessKickDetectionWithBuffer(int32 ChannelIndex);
};

UCLASS(BlueprintType)
class UDreamMusicPlayerExpansion_AudioAnalysisUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void ModifyAudioSoundBus(UAudioBus* InBus, USoundWave* InSoundWave);
};
