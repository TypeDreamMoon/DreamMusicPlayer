// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "UObject/Object.h"
#include "DreamMusicPlayerExpansion.generated.h"

class UDreamMusicData;
class UDreamMusicPlayerComponent;

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Dream Music Player Expansion")
	UDreamMusicPlayerComponent* MusicPlayerComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Dream Music Player Expansion")
	FDreamMusicLyricTimestamp CurrentTimestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Dream Music Player Expansion")
	FDreamMusicDataStruct CurrentMusicData;

public:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent);
	virtual void Tick(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime);
	virtual void ChangeMusic(const FDreamMusicDataStruct& InData);
	virtual void MusicStart();
	virtual void MusicStop();
	virtual void MusicPause();
	virtual void MusicUnPause();
	virtual void MusicEnd();
	virtual void UnbindDelegates();
	virtual void Deinitialize();

protected:
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Initialize")
	void BP_Initialize(UDreamMusicPlayerComponent* InComponent);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Tick")
	void BP_Tick(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Change Music")
	void BP_ChangeMusic(const FDreamMusicDataStruct& InData);

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
