// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_ThemeColors.h"
#include "DreamMusicPlayerDebugLog.h"
#include "AsyncAction/DreamAsyncAction_KMeansTexture.h"
#include "Classes/DreamMusicPlayerComponent.h"

void UDreamMusicPlayerExpansion_ThemeColors::ExtractCoverThemeColors(int32 ClusterCount, int32 MaxIterations)
{
	if (!MusicPlayerComponent->Cover || !MusicPlayerComponent->Cover->IsValidLowLevel())
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("No valid cover image available for theme color extraction"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Clean up existing task more safely
	if (CurrentKMeansTask)
	{
		if (CurrentKMeansTask->IsValidLowLevel() && !CurrentKMeansTask->IsCompletedOrCancelled())
		{
			DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Cancelling previous K-Means task before starting new one"));
			CurrentKMeansTask->Cancel();
		}
		CurrentKMeansTask = nullptr;
	}

	LoadAssetAsync(CurrentMusicData.Information.Cover.ToSoftObjectPath().GetAssetPath(),
	               FLoadAssetAsyncDelegate::CreateLambda(
		               [this, ClusterCount, MaxIterations]
	               (const FTopLevelAssetPath&, UObject* Object, EAsyncLoadingResult::Type)
		               {
			               ExtractTextureThemeColors(
				               Cast<UTexture2D>(Object),
				               ClusterCount,
				               MaxIterations);
		               }));
}


void UDreamMusicPlayerExpansion_ThemeColors::ExtractTextureThemeColors(UTexture2D* Texture, int32 ClusterCount,
                                                                       int32 MaxIterations)
{
	if (!Texture || !Texture->IsValidLowLevel())
	{
		DMP_LOG_DEBUG_EXPANSION(
			Warning, TEXT("Invalid texture for theme color extraction - Texture is null or invalid"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Enhanced texture validation
	const FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() == 0)
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Invalid texture for theme color extraction - No platform data or mips"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	const FTexture2DMipMap& MipMap = PlatformData->Mips[0];
	if (MipMap.BulkData.GetBulkDataSize() == 0)
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Invalid texture for theme color extraction - Empty bulk data"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Clean up existing task safely
	if (CurrentKMeansTask)
	{
		if (CurrentKMeansTask->IsValidLowLevel() && !CurrentKMeansTask->IsCompletedOrCancelled())
		{
			DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Cancelling existing K-Means task"));
			CurrentKMeansTask->Cancel();
		}
		CurrentKMeansTask = nullptr;
	}

	// Create new task with clamped parameters
	CurrentKMeansTask = UDreamAsyncAction_KMeansTexture::KMeansTextureAnalysis(
		Texture,
		FMath::Clamp(ClusterCount, 1, CoverThemeColorCount),
		FMath::Clamp(MaxIterations, 1, MaxIterationsCount),
		FMath::Clamp(SampleRate, 0.1f, 1.0f),
		bIgnoreTransparent,
		FMath::Clamp(AlphaThreshold, 0.0f, 1.0f)
	);

	if (CurrentKMeansTask && CurrentKMeansTask->IsValidLowLevel())
	{
		DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Creating K-Means analysis task for texture: %s (Size: %dx%d, Format: %d)"),
		                        *Texture->GetName(), Texture->GetSizeX(), Texture->GetSizeY(),
		                        (int)Texture->GetPixelFormat());

		CurrentKMeansTask->OnCompleted.
		                   AddDynamic(this, &UDreamMusicPlayerExpansion_ThemeColors::OnThemeColorsExtracted);
		CurrentKMeansTask->Activate();

		DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Started theme color extraction for texture: %s"), *Texture->GetName());
	}
	else
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Failed to create K-Means analysis task for texture: %s"),
		                        *Texture->GetName());
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
	}
}

void UDreamMusicPlayerExpansion_ThemeColors::BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData)
{
	ExtractCoverThemeColors(CoverThemeColorCount, MaxIterationsCount);
}

void UDreamMusicPlayerExpansion_ThemeColors::BP_Deinitialize_Implementation()
{
	if (CurrentKMeansTask && CurrentKMeansTask->IsValidLowLevel())
	{
		CurrentKMeansTask->Cancel();
		CurrentKMeansTask = nullptr;
	}
}

void UDreamMusicPlayerExpansion_ThemeColors::OnThemeColorsExtracted(const TArray<FKMeansColorCluster>& ColorClusters,
                                                                    bool bSuccess)
{
	// Safely clear the current task reference
	UDreamAsyncAction_KMeansTexture* CompletedTask = CurrentKMeansTask;
	CurrentKMeansTask = nullptr;

	if (bSuccess && ColorClusters.Num() > 0)
	{
		DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Theme color extraction completed successfully. Found %d colors"),
		                        ColorClusters.Num());

		// Log extracted colors for debugging
		for (int32 i = 0; i < ColorClusters.Num(); ++i)
		{
			const FKMeansColorCluster& Cluster = ColorClusters[i];
			DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Color %d: R=%.3f G=%.3f B=%.3f Weight=%.3f PixelCount=%d"),
			                        i, Cluster.Color.R, Cluster.Color.G, Cluster.Color.B, Cluster.Weight,
			                        Cluster.PixelCount);
		}
	}
	else
	{
		// Enhanced error reporting
		if (!bSuccess)
		{
			DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Theme color extraction failed or was cancelled"));
		}
		else
		{
			DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Theme color extraction found no colors"));
		}

		// Additional debugging info if task is available
		if (CompletedTask && CompletedTask->IsValidLowLevel())
		{
			DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Task details - Texture: %s, SampledPixels: %d"),
			                        CompletedTask->GetTargetTexture() ? *CompletedTask->GetTargetTexture()->GetName() :
			                        TEXT("NULL"),
			                        CompletedTask->GetSamplePixelNum());
		}
	}

	OnThemeColorChanged.Broadcast(ColorClusters, bSuccess);
}
