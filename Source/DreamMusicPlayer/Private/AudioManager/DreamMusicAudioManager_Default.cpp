// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioManager/DreamMusicAudioManager_Default.h"

#include "Components/AudioComponent.h"

UAudioComponent* UDreamMusicAudioManager_Default::GetAudioComponent()
{
	return AudioComponent;
}

void UDreamMusicAudioManager_Default::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	AudioComponent = NewObject<UAudioComponent>(GetOwner(), FName("DMP_AudioComponent"));
}

bool UDreamMusicAudioManager_Default::IsPlaying() const
{
	return AudioComponent->IsPlaying();
}

void UDreamMusicAudioManager_Default::Music_Play(float InTime)
{
	AudioComponent->Play(InTime);
}

void UDreamMusicAudioManager_Default::Music_Stop()
{
	AudioComponent->Stop();
}

void UDreamMusicAudioManager_Default::Music_Pause()
{
	AudioComponent->SetPaused(true);
}

void UDreamMusicAudioManager_Default::Music_UnPause()
{
	AudioComponent->SetPaused(false);
}

void UDreamMusicAudioManager_Default::Music_Start()
{
	Super::Music_Start();
}
