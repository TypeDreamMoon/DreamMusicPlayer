// Fill out your copyright notice in the Description page of Project Settings.


#include "Classes/DreamMusicAudioManager.h"

#include "Classes/DreamMusicPlayerComponent.h"
#include "Components/AudioComponent.h"

void UDreamMusicAudioManager::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	MusicPlayerComponent = InComponent;
	Owner = InComponent->GetOwner();
}

void UDreamMusicAudioManager::Deinitialize()
{
}

bool UDreamMusicAudioManager::IsPlaying() const
{
	return false;
}

void UDreamMusicAudioManager::Tick(const FDreamMusicLyricTimestamp& InTimestamp, float DeltaTime)
{
}

void UDreamMusicAudioManager::Music_Changed(const FDreamMusicDataStruct& InMusicData)
{
}

void UDreamMusicAudioManager::Music_Play(float InTime)
{
}

void UDreamMusicAudioManager::Music_Start()
{
}

void UDreamMusicAudioManager::Music_Stop()
{
}

void UDreamMusicAudioManager::Music_Pause()
{
}

void UDreamMusicAudioManager::Music_UnPause()
{
}

void UDreamMusicAudioManager::Music_End()
{
}

UAudioComponent* UDreamMusicAudioManager::GetAudioComponent()
{
	return nullptr;
}

bool UDreamMusicAudioManager::IsAudioComponentReady(UAudioComponent* Component) const
{
	return Component &&
		Component->IsValidLowLevel() &&
		Component->IsRegistered() &&
		!Component->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed);
}
