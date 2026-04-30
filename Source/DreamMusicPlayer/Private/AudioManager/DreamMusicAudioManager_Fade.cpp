#include "AudioManager/DreamMusicAudioManager_Fade.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "DreamMusicPlayerDebugLog.h"
#include "Components/AudioComponent.h"

void UDreamMusicAudioManager_Fade::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	Super::Initialize(InComponent);

	AudioTrackA = CreateAudioComponent(TEXT("MusicAudioTrack_A"));
	AudioTrackB = CreateAudioComponent(TEXT("MusicAudioTrack_B"));
	
	// 默认 A 为主轨道
	bIsTrackA_Active = true;
}

UAudioComponent* UDreamMusicAudioManager_Fade::CreateAudioComponent(FName Name)
{
	UAudioComponent* NewComp = NewObject<UAudioComponent>(GetOwner(), Name);
	if (NewComp)
	{
		NewComp->SetupAttachment(GetOwner()->GetRootComponent());
		NewComp->bAutoActivate = false;
		
		if (MusicPlayerComponent && MusicPlayerComponent->SoundClass)
		{
			NewComp->SoundClassOverride = MusicPlayerComponent->SoundClass;
		}
		NewComp->RegisterComponent();
	}
	return NewComp;
}

void UDreamMusicAudioManager_Fade::Deinitialize()
{
	Super::Deinitialize();

	if (GWorld)
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	if (AudioTrackA) AudioTrackA->Stop();
	if (AudioTrackB) AudioTrackB->Stop();
}

void UDreamMusicAudioManager_Fade::Music_Changed(const FDreamMusicData& InMusicData)
{
	// 这里的逻辑是：准备下一首音乐的数据到“次要”轨道，不影响当前播放的“主要”轨道
	UAudioComponent* NextComp = GetSecondaryComponent();
	DMP_LOG(Log, TEXT("%s"), *NextComp->GetName())
	if (!NextComp) return;

	USoundBase* NewSound = nullptr;
	if (InMusicData.CachedMusic != nullptr)
	{
		NewSound = InMusicData.CachedMusic.Get();
	}
	else
	{
		if (InMusicData.Music.IsValid())
		{
			NewSound = InMusicData.Music.Get();
		}
		else if (!InMusicData.Music.IsNull())
		{
			NewSound = InMusicData.Music.LoadSynchronous();
		}
	}

	NextComp->SetSound(NewSound);
}

void UDreamMusicAudioManager_Fade::Music_Play(float InTime)
{
	// 此时，SecondaryComponent 已经装载好了新音乐（在 Music_Changed 中）
	UAudioComponent* NewTrack = GetSecondaryComponent();
	DMP_LOG(Log, TEXT("%s"), *NewTrack->GetName())
	UAudioComponent* OldTrack = GetPrimaryComponent();

	if (!NewTrack) return;

	// 1. 清理之前的淡出 Timer，防止意外停止
	if (GWorld)
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	// 2. 播放新轨道 (Fade In)
	float FadeInTime = FadeAudioSetting.bEnableFadeAudio ? FadeAudioSetting.FadeInDuration : 0.0f;
	
	// 如果需要淡入且是从头播放
	if (FadeInTime > 0.0f && InTime < KINDA_SMALL_NUMBER)
	{
		NewTrack->Sound->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
		// NewTrack->SetVolumeMultiplier(0.0f); // 确保从0开始
		// NewTrack->Play(InTime);
		NewTrack->FadeIn(FadeInTime, Volume, InTime);
	}
	else
	{
		NewTrack->SetVolumeMultiplier(Volume);
		NewTrack->Play(InTime);
	}

	// 3. 停止旧轨道 (Fade Out)
	if (OldTrack && OldTrack->IsPlaying())
	{
		float FadeOutTime = FadeAudioSetting.bEnableFadeAudio ? FadeAudioSetting.FadeOutDuration : 0.0f;
		if (FadeOutTime > 0.0f)
		{
			OldTrack->FadeOut(FadeOutTime, 0.0f);
		}
		else
		{
			OldTrack->Stop();
		}
	}

	// 4. 关键：交换身份。现在新的轨道变成了主轨道。
	SwapActiveTrack();
}

void UDreamMusicAudioManager_Fade::Music_Stop()
{
	// 立即停止所有
	if (GWorld) GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	
	if (AudioTrackA) AudioTrackA->Stop();
	if (AudioTrackB) AudioTrackB->Stop();
}

void UDreamMusicAudioManager_Fade::Music_Pause()
{
	GetPrimaryComponent()->SetPaused(true);
}

void UDreamMusicAudioManager_Fade::Music_UnPause()
{
	GetPrimaryComponent()->SetPaused(false);
}

void UDreamMusicAudioManager_Fade::Music_End()
{
	// 这是 Component 调用的“结束当前音乐”（例如切歌前或者播放完毕）
	// 执行淡出停止
	
	if (GWorld) GWorld->GetTimerManager().ClearTimer(StopTimerHandle);

	UAudioComponent* ActiveComp = GetPrimaryComponent();
	if (!ActiveComp || !ActiveComp->IsPlaying()) return;

	float FadeOutTime = (FadeAudioSetting.bEnableFadeAudio) ? FadeAudioSetting.FadeOutDuration : 0.0f;

	if (FadeOutTime > 0.0f)
	{
		ActiveComp->FadeOut(FadeOutTime, 0.0f);

		// 使用 WeakPtr 保护 Timer 回调
		TWeakObjectPtr<UAudioComponent> WeakComp(ActiveComp);
		if (GWorld)
		{
			GWorld->GetTimerManager().SetTimer(StopTimerHandle, [WeakComp]()
			{
				if (UAudioComponent* StrongComp = WeakComp.Get())
				{
					StrongComp->Stop();
				}
			}, FadeOutTime, false);
		}
	}
	else
	{
		ActiveComp->Stop();
	}
}

void UDreamMusicAudioManager_Fade::SetVolume(float InVolume)
{
	Super::SetVolume(InVolume);
	// 实时调整两个轨道的音量乘数 (注意：FadeIn/Out 会覆盖这个，所以仅在稳定播放时有效)
	// 如果正在 Fade，直接设值可能会打断 Fade 曲线，这里简单处理
	if (AudioTrackA) AudioTrackA->SetVolumeMultiplier(InVolume);
	if (AudioTrackB) AudioTrackB->SetVolumeMultiplier(InVolume);
}

UAudioComponent* UDreamMusicAudioManager_Fade::GetAudioComponent() const
{
	// 总是返回当前的主轨道
	return GetPrimaryComponent();
}

UAudioComponent* UDreamMusicAudioManager_Fade::GetPrimaryComponent() const
{
	return bIsTrackA_Active ? AudioTrackA : AudioTrackB;
}

UAudioComponent* UDreamMusicAudioManager_Fade::GetSecondaryComponent() const
{
	return bIsTrackA_Active ? AudioTrackB : AudioTrackA;
}

void UDreamMusicAudioManager_Fade::SwapActiveTrack()
{
	bIsTrackA_Active = !bIsTrackA_Active;
}