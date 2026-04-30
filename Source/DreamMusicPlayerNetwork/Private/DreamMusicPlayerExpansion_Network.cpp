// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerExpansion_Network.h"

#include "Classes/DreamMusicPlayerComponent.h"
#include "DreamMusicDownloader.h"
#include "DreamMusicPlayerLog.h"

void UDreamMusicPlayerExpansion_Network::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	Super::Initialize(InComponent);
	Downloader = UDreamMusicDownloader::CreateDownloader(this);
	Downloader->OnComplete.AddDynamic(this, &UDreamMusicPlayerExpansion_Network::OnMusicDownloadComplete);
}

void UDreamMusicPlayerExpansion_Network::SetMusicData(const FDreamMusicData& InData)
{
	// 1. 如果不是网络音乐，忽略
	if (InData.MusicType != EDreamMusicPlayerMusicType::Network)
	{
		return;
	}

	// 2. 如果 URL 为空，报错
	if (InData.MusicURL.IsEmpty())
	{
		DMP_LOG(Error, TEXT("Network Music URL is Empty!"));
		return;
	}
	
	// 3. 如果正在下载，取消下载
	if (Downloader->CurrentStatus == EDreamDownloadStatus::Downloading || Downloader->CurrentStatus == EDreamDownloadStatus::Decoding)
	{
		Downloader->Cancel();
	}

	// 3. 开始下载
	DMP_LOG(Log, TEXT("Starting Network Music Download: %s"), *InData.MusicURL);
	Downloader->DownloadAndPrepare(InData.MusicURL);
}

void UDreamMusicPlayerExpansion_Network::OnMusicDownloadComplete(USoundWaveProcedural* SoundWave, const TArray<float>& PcmData, bool bSuccess)
{
	if (!bSuccess || !SoundWave || !MusicPlayerComponent)
	{
		DMP_LOG(Error, TEXT("Network Music Download Failed!"));
		return;
	}

	DMP_LOG(Log, TEXT("Network Music Download Success. Duration: %f"), SoundWave->Duration);

	// 1. 更新主组件的 SoundWave
	MusicPlayerComponent->CurrentMusicData.CachedMusic = SoundWave;
	// 关键：更新组件内部的 MusicDuration 状态，因为之前的 StartMusic 可能因为资源无效将其置为 0
	MusicPlayerComponent->CurrentMusicDuration = SoundWave->Duration;

	// 3. 重新触发播放
	// 由于我们已经在 ChangeMusic 阶段切换了 Data，现在只需让 AudioManager 播放新的 SoundWave
	// 并且通知各个扩展 StartMusic
	
	MusicPlayerComponent->OnMusicLoaded(true);
}
