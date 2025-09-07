// Fill out your copyright notice in the Description page of Project Settings.


#include "Classes/DreamMusicPlayerExpansion.h"

void UDreamMusicPlayerExpansion::BP_Deinitialize_Implementation()
{
}

void UDreamMusicPlayerExpansion::BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData)
{
}

void UDreamMusicPlayerExpansion::BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
}

void UDreamMusicPlayerExpansion::BP_Initialize_Implementation(UDreamMusicPlayerComponent* InComponent)
{
}

void UDreamMusicPlayerExpansion::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	MusicPlayerComponent = InComponent;
	BP_Initialize(InComponent);
}

void UDreamMusicPlayerExpansion::Tick(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
	CurrentTimestamp = InTimestamp;
	BP_Tick(InTimestamp, InDeltaTime);
}

void UDreamMusicPlayerExpansion::ChangeMusic(const FDreamMusicDataStruct& InData)
{
	CurrentMusicData = InData;
	BP_ChangeMusic(InData);
}

void UDreamMusicPlayerExpansion::MusicStart()
{
	BP_MusicStart();
}

void UDreamMusicPlayerExpansion::MusicStop()
{
	BP_MusicStop();
}

void UDreamMusicPlayerExpansion::MusicPause()
{
	BP_MusicPause();
}

void UDreamMusicPlayerExpansion::MusicUnPause()
{
	BP_MusicUnPause();
}

void UDreamMusicPlayerExpansion::MusicEnd()
{
	BP_MusicEnd();
}

void UDreamMusicPlayerExpansion::UnbindDelegates()
{
	BP_UnbindDelegates();
}

void UDreamMusicPlayerExpansion::Deinitialize()
{
	UnbindDelegates();
	BP_Deinitialize();
}

void UDreamMusicPlayerExpansion::BP_MusicEnd_Implementation()
{
}

void UDreamMusicPlayerExpansion::BP_MusicUnPause_Implementation()
{
}

void UDreamMusicPlayerExpansion::BP_MusicPause_Implementation()
{
}

void UDreamMusicPlayerExpansion::BP_MusicStop_Implementation()
{
}

void UDreamMusicPlayerExpansion::BP_MusicStart_Implementation()
{
}
