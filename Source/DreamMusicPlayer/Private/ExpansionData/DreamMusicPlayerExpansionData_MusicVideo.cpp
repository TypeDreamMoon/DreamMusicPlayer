// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpansionData/DreamMusicPlayerExpansionData_MusicVideo.h"

UBaseMediaSource* FDreamMusicPlayerExpansionData_MusicVideo_Define::GetMediaSource() const
{
	return MediaSource;
}

UBaseMediaSource* UDreamMusicPlayerExpansionData_MusicVideo::GetMediaSource() const
{
	return MusicVideo.GetMediaSource();
}
