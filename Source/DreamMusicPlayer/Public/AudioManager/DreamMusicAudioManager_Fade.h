// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicAudioManager.h"
#include "DreamMusicAudioManager_Fade.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Fade")
class DREAMMUSICPLAYER_API UDreamMusicAudioManager_Fade : public UDreamMusicAudioManager
{
	GENERATED_BODY()

public:
	// A Audio Component (Track 1)
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioTrackA = nullptr;

	// B Audio Component (Track 2)
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioTrackB = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FDreamMusicPlayerFadeAudioSetting FadeAudioSetting;

public:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent) override;
	virtual void Deinitialize() override;

	// 核心逻辑接口
	virtual void Music_Changed(const FDreamMusicData& InMusicData) override;
	virtual void Music_Play(float InTime = 0.f) override; // 执行淡入淡出
	virtual void Music_Stop() override;
	virtual void Music_Pause() override;
	virtual void Music_UnPause() override;
	virtual void Music_End() override; // 触发淡出停止
	virtual void SetVolume(float InVolume) override;
	virtual UAudioComponent* GetAudioComponent() const override;

protected:
	/** 辅助函数：创建统一配置的 AudioComponent */
	UAudioComponent* CreateAudioComponent(FName Name);

	/** 获取当前主要播放的组件 */
	UAudioComponent* GetPrimaryComponent() const;

	/** 获取用于准备下一首的组件（后台组件） */
	UAudioComponent* GetSecondaryComponent() const;

	/** 交换主次组件标记 */
	void SwapActiveTrack();

protected:
	// 标记 AudioTrackA 是否为当前主轨道
	UPROPERTY(VisibleInstanceOnly, Category = "State")
	bool bIsTrackA_Active = true;

private:
	FTimerHandle StopTimerHandle;
};
