// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicData.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamMusicTimestamp.h"
#include "UObject/Object.h"
#include "DreamMusicPlayerExpansion.generated.h"

class UDreamMusicDataAsset;
class UDreamMusicPlayerComponent;

/**
 * 
 */
UCLASS(EditInlineNew, Abstract, Blueprintable, BlueprintType)
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Dream Music Player Expansion")
	UDreamMusicPlayerComponent* MusicPlayerComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Dream Music Player Expansion")
	FDreamMusicTimestamp CurrentTimestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Dream Music Player Expansion")
	FDreamMusicData CurrentMusicData;

public:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent);
	virtual void Tick(const FDreamMusicTimestamp& InTimestamp, float InDeltaTime);
	virtual void SetMusicData(const FDreamMusicData& InData);
	virtual void ChangeMusic(const FDreamMusicData& InData);
	virtual void MusicSetPercent(float InPercent);
	virtual void MusicStart();
	virtual void MusicStop();
	virtual void MusicPause();
	virtual void MusicUnPause();
	virtual void MusicEnd();
	virtual void UnbindDelegates();
	virtual void Deinitialize();
	// Whether the extension module supports streaming media playback
	virtual bool SupportStream() const PURE_VIRTUAL(SupportStream, { return false; });
	// Whether the extension module supports networking
	virtual bool SupportNetworking() const PURE_VIRTUAL(SupportNetworking, { return false; });

protected:
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Initialize")
	void BP_Initialize(UDreamMusicPlayerComponent* InComponent);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Tick")
	void BP_Tick(const FDreamMusicTimestamp& InTimestamp, float InDeltaTime);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Change Music")
	void BP_ChangeMusic(const FDreamMusicData& InData);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Music Set Progress")
	void BP_MusicSetPercent(float InPercent);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Music Start")
	void BP_MusicStart();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Music Stop")
	void BP_MusicStop();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Music Pause")
	void BP_MusicPause();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Music UnPause")
	void BP_MusicUnPause();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Music End")
	void BP_MusicEnd();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Deinitialize")
	void BP_Deinitialize();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Unbind Delegates")
	void BP_UnbindDelegates();
};
