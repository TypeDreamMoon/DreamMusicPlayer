// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/DreamMusicPlayerDelegateWidget_LyricExpansion.h"

#include "Classes/DreamMusicPlayerComponent.h"
#include "Expansion/DreamMusicPlayerExpansion_Lyric.h"

void UDreamMusicPlayerDelegateWidget_LyricExpansion::InitializeWidget(UDreamMusicPlayerComponent* InComponent)
{
	if (!InComponent)
	{
		return;
	}

	Super::InitializeWidget(InComponent);

	UDreamMusicPlayerExpansion_Lyric* LyricExpansion = InComponent->GetExpansion<UDreamMusicPlayerExpansion_Lyric>();

	if (!LyricExpansion)
	{
		return;
	}

	LyricExpansion->OnLyricListChanged.AddDynamic(this, &UDreamMusicPlayerDelegateWidget_LyricExpansion::BP_OnLyricListChanged);
	LyricExpansion->OnLyricChanged.AddDynamic(this, &UDreamMusicPlayerDelegateWidget_LyricExpansion::BP_OnLyricChanged);
}

void UDreamMusicPlayerDelegateWidget_LyricExpansion::NativeDestruct()
{
	if (GetMusicPlayerComponent())
	{
		if (GetLyricExpansion()->OnLyricListChanged.IsBound())
			GetLyricExpansion()->OnLyricListChanged.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget_LyricExpansion::BP_OnLyricListChanged);
		if (GetLyricExpansion()->OnLyricChanged.IsBound())
			GetLyricExpansion()->OnLyricChanged.RemoveDynamic(this, &UDreamMusicPlayerDelegateWidget_LyricExpansion::BP_OnLyricChanged);
	}

	Super::NativeDestruct();
}

UDreamMusicPlayerExpansion_Lyric* UDreamMusicPlayerDelegateWidget_LyricExpansion::GetLyricExpansion() const
{
	return GetMusicPlayerComponent()->GetExpansion<UDreamMusicPlayerExpansion_Lyric>();
}

void UDreamMusicPlayerDelegateWidget_LyricExpansion::BP_OnLyricChanged_Implementation(FDreamMusicLyric InLyric, int Index)
{
}

void UDreamMusicPlayerDelegateWidget_LyricExpansion::BP_OnLyricListChanged_Implementation(const TArray<FDreamMusicLyric>& InLyricList)
{
}
