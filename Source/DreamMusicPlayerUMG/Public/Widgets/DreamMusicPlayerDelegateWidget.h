// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Blueprint/UserWidget.h"
#include "DreamMusicPlayerDelegateWidget.generated.h"

struct FDreamMusicDataStruct;
class UDreamMusicPlayerComponent;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class DREAMMUSICPLAYERUMG_API UDreamMusicPlayerDelegateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayerDelegateWidget")
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent);

	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnInitialize"))
	void BP_OnInitialize(UDreamMusicPlayerComponent* InComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicDataChanged"))
	void BP_MusicDataChanged(FDreamMusicDataStruct InData);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicDataListChanged"))
	void BP_MusicDataListChanged(TArray<FDreamMusicDataStruct> InData);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicPlay"))
	void BP_MusicPlay(FDreamMusicDataStruct InData);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicPause"))
	void BP_MusicPause();

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicUnPause"))
	void BP_MusicUnPause();

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicEnd"))
	void BP_MusicEnd();

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnMusicTick"))
	void BP_MusicTick(float InTime);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnPlayStateChanged"))
	void BP_PlayStateChanged(EDreamMusicPlayerPlayState InPlayState);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnPlayModeChanged"))
	void BP_PlayModeChanged(EDreamMusicPlayerPlayMode InPlayMode);

	UFUNCTION(BlueprintNativeEvent, Category = "DreamMusicPlayerDelegateWidget", meta = (DisplayName = "OnExtensionInitializedCompleted"))
	void BP_ExtensionInitializedCompleted();

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayerDelegateWidget")
	UDreamMusicPlayerComponent* GetMusicPlayerComponent() const;

protected:
	TWeakObjectPtr<UDreamMusicPlayerComponent> MusicPlayerComponent;
};
