// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManager/DreamMusicAudioManager_Fade.h"
#include "Classes/DreamMusicPlayerComponent.h"

#include "DreamMusicPlayerDebugLog.h"
#include "Components/AudioComponent.h"

void UDreamMusicAudioManager_Fade::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	Super::Initialize(InComponent);

	SubAudioComponentA = NewObject<UAudioComponent>(GetOwner(), TEXT("MusicPlayerAudioComponentA"));
	if (SubAudioComponentA)
	{
		SubAudioComponentA->SetupAttachment(GetOwner()->GetRootComponent());
		SubAudioComponentA->bAutoActivate = false; // Prevent auto-activation
		if (MusicPlayerComponent->SoundClass)
		{
			SubAudioComponentA->SoundClassOverride = MusicPlayerComponent->SoundClass;
		}
		SubAudioComponentA->RegisterComponent();
	}

	SubAudioComponentB = NewObject<UAudioComponent>(GetOwner(), TEXT("MusicPlayerAudioComponentB"));
	if (SubAudioComponentB)
	{
		SubAudioComponentB->SetupAttachment(GetOwner()->GetRootComponent());
		SubAudioComponentB->bAutoActivate = false; // Prevent auto-activation
		if (MusicPlayerComponent->SoundClass)
		{
			SubAudioComponentB->SoundClassOverride = MusicPlayerComponent->SoundClass;
		}
		SubAudioComponentB->RegisterComponent();
	}
}

void UDreamMusicAudioManager_Fade::Deinitialize()
{
	Super::Deinitialize();

	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}
	
	if (SubAudioComponentA && SubAudioComponentA->IsValidLowLevel())
	{
		SubAudioComponentA->Stop();
	}
	if (SubAudioComponentB && SubAudioComponentB->IsValidLowLevel())
	{
		SubAudioComponentB->Stop();
	}
}

void UDreamMusicAudioManager_Fade::Music_Start()
{
	Super::Music_Start();

	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	UAudioComponent* ActiveComponent = GetAudioComponent();
	if (!ActiveComponent)
	{
		DMP_LOG_DEBUG_EXPANSION(Error, TEXT("No valid audio component available"));
		return;
	}
	
	// Set volume to 0 before playing if fade-in is enabled
	if (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeInDuration > 0.0f)
	{
		ActiveComponent->SetVolumeMultiplier(0.0f);
	}

	ActiveComponent->Play();

	// Apply fade in
	if (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeInDuration > 0.0f)
	{
		ActiveComponent->FadeIn(FadeAudioSetting.FadeInDuration, 1.0f);
	}

	ActiveComponent->SetSound(MusicPlayerComponent->SoundWave);
}

void UDreamMusicAudioManager_Fade::Music_End()
{
	Super::Music_End();

	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	UAudioComponent* ActiveComponent = GetAudioComponent();
	
	// Calculate fade out duration
	float FadeOutDuration = (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeOutDuration > 0.0f)
								? FadeAudioSetting.FadeOutDuration
								: 0.0f;

	// Start fade out
	if (FadeOutDuration > 0.0f)
	{
		ActiveComponent->FadeOut(FadeOutDuration, 0.0f);

		// Schedule stop after fade completes
		if (GWorld)
		{
			GWorld->GetTimerManager().SetTimer(
				StopTimerHandle,
				[this, ActiveComponent]()
				{
					if (ActiveComponent && ActiveComponent->IsValidLowLevel())
					{
						ActiveComponent->Stop();
					}
				},
				FadeOutDuration,
				false
			);
		}
	}
	else
	{
		// Stop immediately if no fade
		ActiveComponent->Stop();
	}
}

UAudioComponent* UDreamMusicAudioManager_Fade::GetAudioComponent()
{
	return GetActiveAudioComponent();
}

UAudioComponent* UDreamMusicAudioManager_Fade::GetActiveAudioComponent() const
{
	UAudioComponent* Component = CurrentActiveAudioComponent ? SubAudioComponentB : SubAudioComponentA;
	return IsAudioComponentReady(Component) ? Component : nullptr;
}


UAudioComponent* UDreamMusicAudioManager_Fade::GetLastActiveAudioComponent() const
{
	UAudioComponent* Component = CurrentActiveAudioComponent ? SubAudioComponentA : SubAudioComponentB;
	return IsAudioComponentReady(Component) ? Component : nullptr;
}

bool UDreamMusicAudioManager_Fade::ToggleActiveAudioComponent()
{
	CurrentActiveAudioComponent = !CurrentActiveAudioComponent;
	DMP_LOG(Log, TEXT("Toggle Active Audio Component : %d"), CurrentActiveAudioComponent)
	return CurrentActiveAudioComponent;
}