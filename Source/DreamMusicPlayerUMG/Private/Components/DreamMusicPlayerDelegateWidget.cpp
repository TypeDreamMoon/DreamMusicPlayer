// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DreamMusicPlayerDelegateWidget.h"

#include "Classes/DreamMusicPlayerComponent.h"

void UDreamMusicPlayerDelegateWidget::InitializeWidget(UDreamMusicPlayerComponent* InComponent)
{
	if (!InComponent)
	{
		return;
	}

	MusicPlayerComponent = InComponent;
	BP_OnInitialize(InComponent);

	InComponent->OnMusicDataChanged.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicDataChanged);
	InComponent->OnMusicDataListChanged.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicDataListChanged);
	InComponent->OnMusicPlay.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicPlay);
	InComponent->OnMusicPause.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicPause);
	InComponent->OnMusicUnPause.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicUnPause);
	InComponent->OnMusicTick.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicTick);
	InComponent->OnMusicEnd.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicEnd);
	InComponent->OnPlayModeChanged.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_PlayModeChanged);
	InComponent->OnPlayStateChanged.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_PlayStateChanged);
	InComponent->OnExtensionInitializedCompleted.AddDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_ExtensionInitializedCompleted);
}

void UDreamMusicPlayerDelegateWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (UDreamMusicPlayerComponent* DMP = GetMusicPlayerComponent())
	{
		if (DMP->OnMusicDataChanged.IsBound())
			DMP->OnMusicDataChanged.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicDataChanged);
		if (DMP->OnMusicDataListChanged.IsBound())
			DMP->OnMusicDataListChanged.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicDataListChanged);
		if (DMP->OnMusicPlay.IsBound())
			DMP->OnMusicPlay.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicPlay);
		if (DMP->OnMusicPause.IsBound())
			DMP->OnMusicPause.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicPause);
		if (DMP->OnMusicUnPause.IsBound())
			DMP->OnMusicUnPause.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicUnPause);
		if (DMP->OnMusicTick.IsBound())
			DMP->OnMusicTick.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicTick);
		if (DMP->OnMusicEnd.IsBound())
			DMP->OnMusicEnd.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_MusicEnd);
		if (DMP->OnPlayModeChanged.IsBound())
			DMP->OnPlayModeChanged.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_PlayModeChanged);
		if (DMP->OnPlayStateChanged.IsBound())
			DMP->OnPlayStateChanged.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_PlayStateChanged);
		if (DMP->OnExtensionInitializedCompleted.IsBound())
			DMP->OnExtensionInitializedCompleted.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget::BP_ExtensionInitializedCompleted);
	}

	MusicPlayerComponent.Reset();
}

void UDreamMusicPlayerDelegateWidget::BP_MusicDataChanged_Implementation(FDreamMusicDataStruct InData)
{
}

void UDreamMusicPlayerDelegateWidget::BP_MusicDataListChanged_Implementation(const TArray<FDreamMusicDataStruct>& InData)
{
}

void UDreamMusicPlayerDelegateWidget::BP_MusicPlay_Implementation(FDreamMusicDataStruct InData)
{
}

void UDreamMusicPlayerDelegateWidget::BP_MusicPause_Implementation()
{
}

void UDreamMusicPlayerDelegateWidget::BP_MusicUnPause_Implementation()
{
}

void UDreamMusicPlayerDelegateWidget::BP_OnInitialize_Implementation(UDreamMusicPlayerComponent* InComponent)
{
}

void UDreamMusicPlayerDelegateWidget::BP_ExtensionInitializedCompleted_Implementation()
{
}

void UDreamMusicPlayerDelegateWidget::BP_PlayModeChanged_Implementation(EDreamMusicPlayerPlayMode InPlayMode)
{
}

void UDreamMusicPlayerDelegateWidget::BP_PlayStateChanged_Implementation(EDreamMusicPlayerPlayState InPlayState)
{
}

void UDreamMusicPlayerDelegateWidget::BP_MusicTick_Implementation(float InTime)
{
}

void UDreamMusicPlayerDelegateWidget::BP_MusicEnd_Implementation()
{
}

UDreamMusicPlayerComponent* UDreamMusicPlayerDelegateWidget::GetMusicPlayerComponent() const
{
	return MusicPlayerComponent.Get();
}
