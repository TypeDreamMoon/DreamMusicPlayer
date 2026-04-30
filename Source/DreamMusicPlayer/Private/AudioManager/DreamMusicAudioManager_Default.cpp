#include "AudioManager/DreamMusicAudioManager_Default.h"
#include "DreamMusicPlayerCommon.h"
#include "Components/AudioComponent.h"
#include "DreamMusicData.h"

UAudioComponent* UDreamMusicAudioManager_Default::GetAudioComponent() const
{
	return AudioComponent;
}

void UDreamMusicAudioManager_Default::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	Super::Initialize(InComponent);
	
	// 创建组件
	AudioComponent = NewObject<UAudioComponent>(GetOwner(), FName("DMP_AudioComponent_Default"));
	if (AudioComponent)
	{
		AudioComponent->RegisterComponent();
	}
}

bool UDreamMusicAudioManager_Default::IsPlaying() const
{
	return AudioComponent && AudioComponent->IsPlaying();
}

void UDreamMusicAudioManager_Default::Music_Changed(const FDreamMusicData& InMusicData)
{
	if (!AudioComponent) return;

	USoundBase* LoadedSound = nullptr;

	// 1. 尝试直接获取（如果已经异步加载完成）
	if (InMusicData.Music.IsValid())
	{
		LoadedSound = InMusicData.Music.Get();
	}
	// 2. 如果没加载，不得不在此处同步加载（建议上层 Component 做异步加载）
	else if (!InMusicData.Music.IsNull())
	{
		LoadedSound = InMusicData.Music.LoadSynchronous();
	}

	// 只有当 Sound 确实变化或当前为空时才设置，避免重置播放进度
	if (AudioComponent->Sound != LoadedSound)
	{
		AudioComponent->SetSound(LoadedSound);
	}
}

void UDreamMusicAudioManager_Default::Music_Play(float InTime)
{
	if (AudioComponent)
	{
		AudioComponent->Play(InTime);
	}
}

void UDreamMusicAudioManager_Default::Music_Stop()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void UDreamMusicAudioManager_Default::Music_Pause()
{
	if (AudioComponent)
	{
		AudioComponent->SetPaused(true);
	}
}

void UDreamMusicAudioManager_Default::Music_UnPause()
{
	if (AudioComponent)
	{
		AudioComponent->SetPaused(false);
	}
}