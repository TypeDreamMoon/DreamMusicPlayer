// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerDelegateWidget.h"
#include "DreamMusicPlayerDelegateWidget_LyricExpansion.generated.h"

struct FDreamMusicLyricGroup;
struct FDreamMusicLyric;
class UDreamMusicPlayerExpansion_Lyric;

/**
 * 
 */
UCLASS(Abstract)
class DREAMMUSICPLAYERUMG_API UDreamMusicPlayerDelegateWidget_LyricExpansion : public UDreamMusicPlayerDelegateWidget
{
	GENERATED_BODY()

public:
	virtual void InitializeWidget(UDreamMusicPlayerComponent* InComponent) override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", Meta = (DisplayName = "On Lyric List Changed"))
	void BP_OnLyricListChanged(const TArray<FDreamMusicLyricGroup>& InLyricList);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", Meta = (DisplayName = "On Lyric Changed"))
	void BP_OnLyricChanged(FDreamMusicLyricGroup InLyric, int Index);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayerDelegateWidget")
	UDreamMusicPlayerExpansion_Lyric* GetLyricExpansion() const;
};
