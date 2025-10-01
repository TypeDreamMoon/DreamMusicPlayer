// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_AudioAnalysis.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "ConstantQNRT.h"
#include "ConstantQNRTFactory.h"
#include "DreamMusicPlayerDebugLog.h"
#include "DreamMusicPlayerLog.h"
#include "LoudnessNRT.h"
#include "LoudnessNRTFactory.h"
#include "Engine/Canvas.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_AudioAnalysis.h"
#include "Kismet/KismetRenderingLibrary.h"

void UDreamMusicPlayerExpansion_AudioAnalysis::GetAudioNrtData(TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR,
                                                               TArray<float>& ConstantNrtAverage,
                                                               float& OutLoudnessValue)
{
	ConstantNrtL = ConstantQDataL;
	ConstantNrtR = ConstantQDataR;
	ConstantNrtAverage = ConstantQDataAverage;
	OutLoudnessValue = LoudnessValue;
}

void UDreamMusicPlayerExpansion_AudioAnalysis::UpdateAudioAnalysisData()
{
	bool bCreateTexture = false;
	
	if (ConstantQ && ConstantQ->IsValidLowLevel() && IsValid(ConstantQ))
	{
		TSharedPtr<const Audio::FConstantQNRTResult, ESPMode::ThreadSafe> ConstantQResult = ConstantQ->GetResult<Audio::FConstantQNRTResult>();

		if (ConstantQResult->IsSortedChronologically())
		{
			bCreateTexture = true;
			
			ConstantQ->GetNormalizedChannelConstantQAtTime(MusicPlayerComponent->CurrentDuration, 0, ConstantQDataL);
			ConstantQ->GetNormalizedChannelConstantQAtTime(MusicPlayerComponent->CurrentDuration, 1, ConstantQDataR);

			ConstantQData.Empty();
			ConstantQDataAverage.Empty();

			auto Avg = [this]()
			{
				for (int i = 0; i < ConstantQData.Num() / 2; ++i)
				{
					ConstantQDataAverage.Add((ConstantQData[i] + ConstantQData[ConstantQData.Num() - i - 1]) / 2.f);
				}
			};

			switch (AverageType)
			{
			case EDreamMusicPlayerExpansion_AudioAnalysis_AverageType::Left:
				ConstantQDataAverage = ConstantQDataL;
				break;
			case EDreamMusicPlayerExpansion_AudioAnalysis_AverageType::Right:
				ConstantQDataAverage = ConstantQDataR;
				break;
			case EDreamMusicPlayerExpansion_AudioAnalysis_AverageType::Left_Right:
				ConstantQData.Append(ConstantQDataL);
				ConstantQData.Append(ConstantQDataR);
				Avg();
				break;
			case EDreamMusicPlayerExpansion_AudioAnalysis_AverageType::Right_Left:
				ConstantQData.Append(ConstantQDataR);
				ConstantQData.Append(ConstantQDataL);
				Avg();
				break;
			}
		}
	}

	if (Loudness && Loudness->IsValidLowLevel() && IsValid(Loudness))
	{
		TSharedPtr<const Audio::FLoudnessNRTResult, ESPMode::ThreadSafe> LoudnessResult = Loudness->GetResult<Audio::FLoudnessNRTResult>();

		if (LoudnessResult->IsSortedChronologically())
		{
			bCreateTexture = true;
			
			Loudness->GetNormalizedLoudnessAtTime(MusicPlayerComponent->CurrentDuration, LoudnessValue);
		}
	}

	if (bEnableCreateAnalysisTexture && bIsCreated && Internal_AnalysisTexture && bCreateTexture)
	{
		// fmt: B=1byte G=1byte R=1byte A=1byte 48=Width(Channel) 1=Height
		uint8* Pixels = BuildPixelArray(ConstantQDataAverage);
		WriteTextureFromPixel(Pixels, Internal_AnalysisTexture);
		delete[] Pixels;
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData)
{
	LoadAudioNrt();
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
	UpdateAudioAnalysisData();
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_MusicStart_Implementation()
{
	if (Internal_AnalysisTexture)
	{
		// Clear
		Internal_AnalysisTexture->MarkAsGarbage();
		Internal_AnalysisTexture = nullptr;
	}

	if (bEnableCreateAnalysisTexture)
	{
		if (UTexture2D* CacheTexture = UTexture2D::CreateTransient(48, 1, PF_B8G8R8A8))
		{
			CacheTexture->MipGenSettings = TMGS_NoMipmaps;
			CacheTexture->Filter = TF_Nearest;
			CacheTexture->CompressionSettings = TC_EditorIcon;

			CacheTexture->LODGroup = TEXTUREGROUP_UI;
			CacheTexture->UpdateResource();

			Internal_AnalysisTexture = CacheTexture;

			bIsCreated = true;

			OnAnalysisTextureLoaded.Broadcast(Internal_AnalysisTexture);
		}
		else
		{
			DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Failed to create texture"));
		}
	}
}

uint8* UDreamMusicPlayerExpansion_AudioAnalysis::BuildPixelArray(const TArray<float>& Data)
{
	if (Data.IsEmpty())
	{
		return nullptr;
	}

	// fmt: B=1byte G=1byte R=1byte A=1byte 48=Width(Channel) 1=Height
	uint8* Pixels = new uint8[48 * 1 * 4];
	for (int32 y = 0; y < 1; ++y)
	{
		for (int32 x = 0; x < 48; ++x)
		{
			int32 PixelIdx = y * 48 + x;
			if (Data.IsValidIndex(PixelIdx))
			{
				FColor Color = FLinearColor(Data[PixelIdx], Data[PixelIdx], Data[PixelIdx], 1.0f).ToFColor(false);
				// BGRA
				Pixels[4 * PixelIdx] = Color.B; // B
				Pixels[4 * PixelIdx + 1] = Color.G; // G
				Pixels[4 * PixelIdx + 2] = Color.R; // R
				Pixels[4 * PixelIdx + 3] = Color.A; // A
			}
			else
			{
				DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Invalid index, %d"), PixelIdx)
				Pixels[4 * PixelIdx] = 0; // B
				Pixels[4 * PixelIdx + 1] = 0; // G
				Pixels[4 * PixelIdx + 2] = 0; // R
				Pixels[4 * PixelIdx + 3] = 0; // A
			}
		}
	}

	return Pixels;
}

void UDreamMusicPlayerExpansion_AudioAnalysis::WriteTextureFromPixel(uint8* PixelArray, UTexture2D* Texture2D)
{
	FTexturePlatformData* PlatformData = Texture2D->GetPlatformData();
	if (!PlatformData || !PlatformData->Mips.IsValidIndex(0))
	{
		DMP_LOG_DEBUG_EXPANSION(Error, TEXT("Invalid platform data"))
		delete[] PixelArray;
	}
	else
	{
		void* TextureData = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

		const int32 Size = sizeof(uint8) * 48 * 1 * 4;

		FMemory::Memcpy(TextureData, PixelArray, Size);

		PlatformData->Mips[0].BulkData.Unlock();

		Texture2D->UpdateResource();
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::LoadAudioNrt()
{
	UDreamMusicPlayerExpansionData_AudioAnalysis* MusicData = CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_AudioAnalysis>();
	if (!MusicData) return;

	FSoftObjectPath CQ = MusicData->ConstantQ;
	FSoftObjectPath LN = MusicData->Loudness;
	if (CQ.IsValid())
		ConstantQ = Cast<UConstantQNRT>(CQ.TryLoad());
	if (LN.IsValid())
		Loudness = Cast<ULoudnessNRT>(LN.TryLoad());
}
