// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_Lyric.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Lyric")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_Lyric : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerLyircListDelegate, const TArray<FDreamMusicLyric>&,
	                                            LyricList);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerLyricDelegate, FDreamMusicLyric, Lyric);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMusicPlayerLyricAndIndexDelegate, FDreamMusicLyric, Lyric, int, Index);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FMusicPlayerLyricAndIndexMulticaseDelegate, FDreamMusicLyric, int);
public:
	// Lyric Offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float LyricOffset = 0.0f;
	
	// Current Music Lyric List
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TArray<FDreamMusicLyric> CurrentMusicLyricList;

	// Current Music Lyric
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FDreamMusicLyric CurrentLyric;

public:
	/**
	 * Lyric List Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Lyric")
	FMusicPlayerLyircListDelegate OnLyricListChanged;

	/**
	 * Lyric Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Lyric")
	FMusicPlayerLyricAndIndexDelegate OnLyricChanged;

	FMusicPlayerLyricAndIndexMulticaseDelegate OnLyricChangedNative;

public:
	/**
	 * Get Current Lyric Line Progress (fallback when no word timings available)
	 * @param InTimestamp Current playback time in seconds
	 * @return Progress information based on line timestamps
	 */
	UFUNCTION(BlueprintPure, Category = "Functions|Lyric")
	FDreamMusicLyricProgress GetCurrentLyricLineProgress(const FDreamMusicLyricTimestamp& InTimestamp) const;

	/**
	* Get Current Lyric Word Progress for regular lyrics
	* @param InTimestamp Current playback time in seconds
	* @return Progress information for word timings
	*/
	UFUNCTION(BlueprintPure, Category = "Functions|Lyric")
	FDreamMusicLyricProgress GetCurrentLyricWordProgress(const FDreamMusicLyricTimestamp& InTimestamp) const;

	/**
	 * Get Current Romanization Word Progress
	 * @param InTimestamp Current playback time in seconds
	 * @return Progress information for romanization word timings
	 */
	UFUNCTION(BlueprintPure, Category = "Functions|Lyric")
	FDreamMusicLyricProgress GetCurrentRomanizationProgress(const FDreamMusicLyricTimestamp& InTimestamp) const;

	/**
	 * Play Music Time From Lyric Timestamp
	 * @param InLyric Lyric
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicWithLyric(FDreamMusicLyric InLyric);

	/**
	 * Initialize Lyric List
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeLyricList();

protected:
	/**
	 * Helper function to calculate word progress
	 * @param InCurrentTime Current playback time in seconds
	 * @param bUseRoma Array of word timings
	 * @return Progress information
	 */
	FDreamMusicLyricProgress CalculateWordProgress(FDreamMusicLyricTimestamp InCurrentTime, bool bUseRoma = false) const;

	/**
	 * Helper function to calculate line progress
	 * @param InCurrentTime Current playback time in seconds
	 * @return Progress information
	 */
	FDreamMusicLyricProgress CalculateLineProgress(FDreamMusicLyricTimestamp InCurrentTime) const;

	/**
	 * Set Current Lyric
	 * @param InLyric New Lyric
	 */
	void SetCurrentLyric(FDreamMusicLyric InLyric);

	mutable int32 CachedCurrentWordIndex = -1;
	mutable FDreamMusicLyricTimestamp LastCalculationTime;
	mutable TArray<int32> WordDurationPrefixSum; // 前缀和数组，提升查找性能
	mutable bool bCacheValid = false;
	mutable bool bLastUseRoma = false;

	void BuildWordDurationCache(bool bUseRoma) const;

	void ClearLyricProgressCache()
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		LastCalculationTime = FDreamMusicLyricTimestamp{};
	}

protected:
	virtual void BP_MusicStart_Implementation() override;
	virtual void BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime) override;
};
