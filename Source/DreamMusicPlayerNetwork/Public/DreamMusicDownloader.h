#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Sound/SoundWaveProcedural.h"
#include "DreamMusicDownloader.generated.h"

// 前向声明
struct FDecodedAudioData;

UENUM(BlueprintType)
enum class EDreamDownloadStatus : uint8
{
	Idle,
	Downloading,
	Decoding,
	Success,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicDownloadProgress, float, Percentage);
// 完成事件：返回 SoundWave 用于播放，PcmDataForAubio 用于分析
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMusicDownloadComplete, USoundWaveProcedural*, SoundWave, const TArray<float>&, PcmDataForAubio, bool, bSuccess);

UCLASS(BlueprintType)
class DREAMMUSICPLAYERNETWORK_API UDreamMusicDownloader : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DreamMusic|Downloader")
	static UDreamMusicDownloader* CreateDownloader(UObject* Outer);

	UFUNCTION(BlueprintCallable, Category = "DreamMusic|Downloader")
	void DownloadAndPrepare(const FString& URL);

	UFUNCTION(BlueprintCallable, Category = "DreamMusic|Downloader")
	void Cancel();

	UPROPERTY(BlueprintAssignable)
	FOnMusicDownloadProgress OnProgress;

	UPROPERTY(BlueprintAssignable)
	FOnMusicDownloadComplete OnComplete;

	UPROPERTY(BlueprintReadOnly)
	EDreamDownloadStatus CurrentStatus = EDreamDownloadStatus::Idle;

	UPROPERTY(BlueprintReadOnly)
	FString LastErrorMessage;

private:
	// HTTP 回调
	void OnRequestProgress(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived);
	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// 解码回调
	void OnDecodeFinished(const FDecodedAudioData& Result);

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;
    
	UPROPERTY()
	USoundWaveProcedural* ResultSoundWave;
};