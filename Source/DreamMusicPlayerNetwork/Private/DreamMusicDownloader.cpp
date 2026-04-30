#include "DreamMusicDownloader.h"

#include "DreamMusicDrLibsDecoder.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Async/Async.h"

UDreamMusicDownloader* UDreamMusicDownloader::CreateDownloader(UObject* Outer)
{
	return NewObject<UDreamMusicDownloader>(Outer);
}

void UDreamMusicDownloader::Cancel()
{
	if (CurrentRequest.IsValid() && CurrentStatus == EDreamDownloadStatus::Downloading)
	{
		CurrentRequest->CancelRequest();
	}
	CurrentStatus = EDreamDownloadStatus::Idle;
}

void UDreamMusicDownloader::DownloadAndPrepare(const FString& URL)
{
	if (CurrentStatus == EDreamDownloadStatus::Downloading || CurrentStatus == EDreamDownloadStatus::Decoding)
	{
		UE_LOG(LogTemp, Warning, TEXT("Downloader busy."));
		return;
	}

	if (URL.IsEmpty())
	{
		LastErrorMessage = TEXT("URL is empty");
		CurrentStatus = EDreamDownloadStatus::Failed;
		OnComplete.Broadcast(nullptr, TArray<float>(), false);
		return;
	}

	CurrentStatus = EDreamDownloadStatus::Downloading;
	LastErrorMessage = TEXT("");

	FHttpModule* Http = &FHttpModule::Get();
	CurrentRequest = Http->CreateRequest();
	CurrentRequest->SetVerb("GET");
	CurrentRequest->SetURL(URL);

	// ★ 关键：根据你上传的 IHttpRequest.h，必须使用 OnRequestProgress64
	CurrentRequest->OnRequestProgress64().BindUObject(this, &UDreamMusicDownloader::OnRequestProgress);
	CurrentRequest->OnProcessRequestComplete().BindUObject(this, &UDreamMusicDownloader::OnRequestComplete);

	CurrentRequest->ProcessRequest();
}

void UDreamMusicDownloader::OnRequestProgress(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived)
{
	if (!Request.IsValid()) return;

	// IHttpResponse 可能还未准备好，需检查
	if (Request->GetStatus() == EHttpRequestStatus::Processing)
	{
		// 注意：这里可能无法获取 ContentLength，取决于服务器是否返回该 Header
		float Percent = 0.0f;
		// 如果 response header 还没回来，GetResponse() 可能是 null
		if (Request->GetResponse().IsValid())
		{
			uint64 TotalSize = Request->GetResponse()->GetContentLength();
			if (TotalSize > 0)
			{
				Percent = (float)BytesReceived / (float)TotalSize;
			}
		}
		OnProgress.Broadcast(Percent);
	}
}

void UDreamMusicDownloader::OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	CurrentRequest.Reset(); // 释放引用

	// 1. 网络层级检查
	if (!bWasSuccessful || !Response.IsValid())
	{
		CurrentStatus = EDreamDownloadStatus::Failed;
		LastErrorMessage = TEXT("Network Connection Failed");
		OnComplete.Broadcast(nullptr, TArray<float>(), false);
		return;
	}

	// 2. HTTP 状态码检查
	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		CurrentStatus = EDreamDownloadStatus::Failed;
		LastErrorMessage = FString::Printf(TEXT("HTTP Error: %d"), Response->GetResponseCode());
		OnComplete.Broadcast(nullptr, TArray<float>(), false);
		return;
	}

	// 3. 数据检查
	const TArray<uint8>& Content = Response->GetContent();
	if (Content.Num() == 0)
	{
		CurrentStatus = EDreamDownloadStatus::Failed;
		LastErrorMessage = TEXT("Downloaded content is empty");
		OnComplete.Broadcast(nullptr, TArray<float>(), false);
		return;
	}

	// 4. 进入解码阶段
	CurrentStatus = EDreamDownloadStatus::Decoding;

	// 启动后台任务：不在主线程解码音频！
	// 深拷贝 Content 数据到 Lambda，防止 Response 被销毁
	TArray<uint8> RawDataCopy = Content;
	TWeakObjectPtr<UDreamMusicDownloader> WeakThis(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, RawDataCopy]()
	{
		// === 后台线程 ===
		FDecodedAudioData DecodeResult = FDreamMusicDrLibsDecoder::DecodeMemory(RawDataCopy.GetData(), RawDataCopy.Num());

		// === 回到主线程 ===
		AsyncTask(ENamedThreads::GameThread, [WeakThis, DecodeResult]()
		{
			if (UDreamMusicDownloader* StrongThis = WeakThis.Get())
			{
				StrongThis->OnDecodeFinished(DecodeResult);
			}
		});
	});
}

void UDreamMusicDownloader::OnDecodeFinished(const FDecodedAudioData& Result)
{
	if (!Result.bSuccess)
	{
		CurrentStatus = EDreamDownloadStatus::Failed;
		LastErrorMessage = FString::Printf(TEXT("Decode Failed: %s"), *Result.ErrorMsg);
		OnComplete.Broadcast(nullptr, TArray<float>(), false);
		return;
	}

	// 创建 SoundWaveProcedural (只能在主线程)
	ResultSoundWave = NewObject<USoundWaveProcedural>();
	ResultSoundWave->SetSampleRate(Result.SampleRate);
	ResultSoundWave->NumChannels = Result.Channels;
	ResultSoundWave->Duration = Result.Duration;

	// 设置为程序化生成，这样它就不会去尝试加载文件
	ResultSoundWave->bProcedural = true;

	// 填充音频队列
	// QueueAudio 需要 const uint8*，所以我们将 int16* 强转，并乘以 sizeof(int16)
	ResultSoundWave->QueueAudio(
		reinterpret_cast<const uint8*>(Result.PCMDataInt16.GetData()),
		Result.PCMDataInt16.Num() * sizeof(int16)
	);

	CurrentStatus = EDreamDownloadStatus::Success;

	UE_LOG(LogTemp, Log, TEXT("Download Success: %.2fs, %d Hz"), Result.Duration, Result.SampleRate);

	// 广播结果：
	// ResultSoundWave -> 直接给 AudioComponent 播放
	// Result.PCMDataFloat -> 直接给 Aubio 分析 (省去了从 SoundWave 再次解压的开销)
	OnComplete.Broadcast(ResultSoundWave, Result.PCMDataFloat, true);
}
