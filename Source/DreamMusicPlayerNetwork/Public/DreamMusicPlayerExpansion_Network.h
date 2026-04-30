// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_Network.generated.h"

class UDreamMusicDownloader;
/**
 * 
 */
UCLASS(DisplayName = "Network Support")
class DREAMMUSICPLAYERNETWORK_API UDreamMusicPlayerExpansion_Network : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	// --- Overrides ---
	virtual bool SupportNetworking() const override { return true; }
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent) override;
	virtual void SetMusicData(const FDreamMusicData& InData) override;

private:
	friend UDreamMusicPlayerComponent;
	
	UPROPERTY()
	UDreamMusicDownloader* Downloader;

	// 下载完成回调
	UFUNCTION()
	void OnMusicDownloadComplete(USoundWaveProcedural* SoundWave, const TArray<float>& PcmData, bool bSuccess);
};
