// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_MusicVideo.h"

#include "MediaPlayer.h"
#include "BaseMediaSource.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_MusicVideo.h"

void UDreamMusicPlayerExpansion_MusicVideo::OnMediaOpenedHandle(FString OpenedUrl)
{
	MediaPlayer->Play();
}

void UDreamMusicPlayerExpansion_MusicVideo::BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData)
{
	if (!InData.HasExpansionData(UDreamMusicPlayerExpansionData_MusicVideo::StaticClass()))
	{
		MediaPlayer->Close();
		return;
	}
		
	UDreamMusicPlayerExpansionData_MusicVideo* MusicVideoData = InData.GetExpansionData<UDreamMusicPlayerExpansionData_MusicVideo>();

	if (MusicVideoData)
	{
		CachedMusicVideoData = MusicVideoData->MusicVideo;
		
		UBaseMediaSource* MediaSource = MusicVideoData->GetMediaSource();

		MediaPlayer->SetLooping(CachedMusicVideoData.bCanLoop);
		
		MediaPlayer->OnMediaOpened.AddDynamic(this, &UDreamMusicPlayerExpansion_MusicVideo::OnMediaOpenedHandle);
		MediaPlayer->OpenSource(MediaSource);
	}
}
