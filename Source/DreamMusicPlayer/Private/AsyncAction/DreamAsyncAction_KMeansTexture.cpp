#include "AsyncAction/DreamAsyncAction_KMeansTexture.h"

#include "DreamMusicPlayerLog.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Async/Async.h"

UDreamAsyncAction_KMeansTexture* UDreamAsyncAction_KMeansTexture::KMeansTextureAnalysis(
	UTexture2D* Texture,
	int32 ClusterCount,
	int32 MaxIterations,
	float SampleRate,
	bool bIgnoreTransparent,
	float AlphaThreshold)
{
	UDreamAsyncAction_KMeansTexture* Action = NewObject<UDreamAsyncAction_KMeansTexture>();
	Action->SourceTexture = Texture;
	Action->NumClusters = FMath::Clamp(ClusterCount, 1, 16);
	Action->MaxIterationsCount = FMath::Clamp(MaxIterations, 1, 1000);
	Action->PixelSampleRate = FMath::Clamp(SampleRate, 0.01f, 1.0f);
	Action->bIgnoreTransparentPixels = bIgnoreTransparent;
	Action->MinAlphaThreshold = FMath::Clamp(AlphaThreshold, 0.0f, 1.0f);
	Action->bIsCancelled = false;

	return Action;
}

void UDreamAsyncAction_KMeansTexture::Activate()
{
	if (!SourceTexture)
	{
		DMP_LOG(Error, TEXT("SourceTexture is null"));
		CompleteTask(false);
		return;
	}

	if (!SourceTexture->GetPlatformData())
	{
		DMP_LOG(Error, TEXT("Texture has no platform data"));
		CompleteTask(false);
		return;
	}

	if (SourceTexture->GetPlatformData()->Mips.Num() == 0)
	{
		DMP_LOG(Error, TEXT("Texture has no mip levels"));
		CompleteTask(false);
		return;
	}

	// Validate texture is ready for reading
	if (SourceTexture->GetPlatformData()->Mips[0].BulkData.GetBulkDataSize() == 0)
	{
		DMP_LOG(Error, TEXT("Texture BulkData is empty"));
		CompleteTask(false);
		return;
	}

	// Execute K-Means on background thread
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
	{
		if (bIsCancelled)
		{
			return;
		}

		ExecuteKMeans();
	});
}

void UDreamAsyncAction_KMeansTexture::Cancel()
{
	bIsCancelled = true;
	Super::Cancel();
}

void UDreamAsyncAction_KMeansTexture::ExecuteKMeans()
{
	DMP_LOG(Log, TEXT("ExecuteKMeans - Starting K-Means analysis"));

	// Sample pixels from texture
	SampleTexturePixels();

	DMP_LOG(Log, TEXT("ExecuteKMeans - After sampling: Cancelled=%s, SampledPixels=%d"),
	        bIsCancelled ? TEXT("true") : TEXT("false"), SampledPixels.Num());

	if (bIsCancelled)
	{
		DMP_LOG(Warning, TEXT("ExecuteKMeans - Task was cancelled during pixel sampling"));
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			CompleteTask(false);
		});
		return;
	}

	if (SampledPixels.Num() == 0)
	{
		DMP_LOG(Warning, TEXT("ExecuteKMeans - No pixels sampled from texture. Cannot perform K-Means clustering."));
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			CompleteTask(false);
		});
		return;
	}

	// Initialize cluster centroids
	InitializeClusters();

	DMP_LOG(Log, TEXT("ExecuteKMeans - Initialized %d clusters"), ClusterCentroids.Num());

	// Run K-Means iterations
	bool bConverged = false;
	int32 Iteration = 0;

	while (!bConverged && Iteration < MaxIterationsCount && !bIsCancelled)
	{
		DMP_LOG(Log, TEXT("ExecuteKMeans - Starting iteration %d"), Iteration + 1);

		// Assign pixels to nearest clusters
		bool bAssignmentsChanged = AssignPixelsToClusters();

		if (bIsCancelled)
		{
			DMP_LOG(Warning, TEXT("ExecuteKMeans - Task cancelled during pixel assignment"));
			return;
		}

		// Update cluster centroids
		UpdateClusterCentroids();

		if (bIsCancelled)
		{
			DMP_LOG(Warning, TEXT("ExecuteKMeans - Task cancelled during centroid update"));
			return;
		}

		// Check for convergence
		bConverged = !bAssignmentsChanged;
		Iteration++;

		DMP_LOG(Log, TEXT("ExecuteKMeans - Completed iteration %d, Converged=%s, AssignmentsChanged=%s"),
		        Iteration, bConverged ? TEXT("true") : TEXT("false"), bAssignmentsChanged ? TEXT("true") : TEXT("false"));
	}

	DMP_LOG(Log, TEXT("ExecuteKMeans - Completed %d iterations, converged: %s"),
	        Iteration, bConverged ? TEXT("true") : TEXT("false"));

	// Return to game thread to complete
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		CompleteTask(!bIsCancelled);
	});
}

void UDreamAsyncAction_KMeansTexture::SampleTexturePixels()
{
	if (!SourceTexture || bIsCancelled)
	{
		DMP_LOG(Warning, TEXT("SampleTexturePixels - Early exit: Texture=%s, Cancelled=%s"),
		        SourceTexture ? *SourceTexture->GetName() : TEXT("NULL"),
		        bIsCancelled ? TEXT("true") : TEXT("false"));
		return;
	}

	const int32 TextureWidth = SourceTexture->GetSizeX();
	const int32 TextureHeight = SourceTexture->GetSizeY();
	const EPixelFormat PixelFormat = SourceTexture->GetPixelFormat();

	DMP_LOG(Log, TEXT("SampleTexturePixels - Texture info: %s, Size=%dx%d, Format=%d"),
	        *SourceTexture->GetName(), TextureWidth, TextureHeight, (int)PixelFormat);

	if (TextureWidth <= 0 || TextureHeight <= 0)
	{
		DMP_LOG(Warning, TEXT("SampleTexturePixels - Invalid texture dimensions: %dx%d"), TextureWidth, TextureHeight);
		return;
	}

	// Ensure platform data is available
	if (!SourceTexture->GetPlatformData() || SourceTexture->GetPlatformData()->Mips.Num() == 0)
	{
		DMP_LOG(Warning, TEXT("SampleTexturePixels - No platform data or mip levels"));
		return;
	}

	FTexture2DMipMap& MipMap = SourceTexture->GetPlatformData()->Mips[0];

	if (MipMap.BulkData.GetBulkDataSize() == 0)
	{
		DMP_LOG(Warning, TEXT("SampleTexturePixels - Empty bulk data"));
		return;
	}

	// Lock the texture data for reading
	void* TextureData = MipMap.BulkData.Lock(LOCK_READ_ONLY);

	DMP_LOG(Error, TEXT("%p"), TextureData);

	if (!TextureData)
	{
		DMP_LOG(Warning, TEXT("SampleTexturePixels - Failed to lock texture data"));
		return;
	}

	SampledPixels.Empty();

	int32 SampleStep = FMath::Max(1, FMath::RoundToInt(1.0f / PixelSampleRate));
	DMP_LOG(Log, TEXT("SampleTexturePixels - Sample rate: %f, Sample step: %d"), PixelSampleRate, SampleStep);

	if (PixelFormat == PF_B8G8R8A8)
	{
		const FColor* PixelData = static_cast<const FColor*>(TextureData);
		DMP_LOG(Error, TEXT("%p"), PixelData);
		int32 TotalPixels = 0;
		int32 SampledCount = 0;

		for (int32 Y = 0; Y < TextureHeight; Y += SampleStep)
		{
			if (bIsCancelled) break;

			for (int32 X = 0; X < TextureWidth; X += SampleStep)
			{
				TotalPixels++;
				if (bIsCancelled) break;

				const FColor& Pixel = PixelData[Y * TextureWidth + X];
				FLinearColor LinearColor = FLinearColor::FromSRGBColor(Pixel);
				DMP_LOG(Error, TEXT("%s"), *Pixel.ToString());
				if (bIgnoreTransparentPixels && LinearColor.A < MinAlphaThreshold)
				{
					continue;
				}

				SampledPixels.Add(LinearColor);
				SampledCount++;
			}
		}

		DMP_LOG(Log, TEXT("SampleTexturePixels - PF_B8G8R8A8: Total checked=%d, Sampled=%d"), TotalPixels, SampledCount);
	}
	else if (PixelFormat == PF_FloatRGBA)
	{
		const FLinearColor* PixelData = static_cast<const FLinearColor*>(TextureData);
		int32 TotalPixels = 0;
		int32 SampledCount = 0;

		for (int32 Y = 0; Y < TextureHeight; Y += SampleStep)
		{
			if (bIsCancelled) break;

			for (int32 X = 0; X < TextureWidth; X += SampleStep)
			{
				TotalPixels++;
				if (bIsCancelled) break;

				const FLinearColor& Pixel = PixelData[Y * TextureWidth + X];

				if (bIgnoreTransparentPixels && Pixel.A < MinAlphaThreshold)
				{
					continue;
				}

				SampledPixels.Add(Pixel);
				SampledCount++;
			}
		}

		DMP_LOG(Log, TEXT("SampleTexturePixels - PF_FloatRGBA: Total checked=%d, Sampled=%d"), TotalPixels, SampledCount);
	}
	else if (PixelFormat == PF_DXT1)
	{
		// DXT1 是一种压缩纹理格式，每个块的大小为 8 字节，表示 4x4 的像素
		const uint8* TextureDataBytes = static_cast<const uint8*>(TextureData);
		int32 TotalPixels = 0;
		int32 SampledCount = 0;

		for (int32 Y = 0; Y < TextureHeight; Y += 4) // 每次步长 4，处理每个 4x4 块
		{
			if (bIsCancelled) break;

			for (int32 X = 0; X < TextureWidth; X += 4)
			{
				TotalPixels++;

				if (bIsCancelled) break;

				// 计算当前块在纹理数据中的偏移量
				int32 BlockIndex = ((Y / 4) * (TextureWidth / 4)) + (X / 4);
				const uint8* BlockData = TextureDataBytes + BlockIndex * 8; // 每个 DXT1 块占 8 字节

				// 解码 DXT1 块
				FLinearColor BlockPixels[16];
				DecodeDXT1Block(BlockData, BlockPixels);

				// 将解码的像素加入到 SampledPixels 数组中
				for (int32 PixelY = 0; PixelY < 4; ++PixelY)
				{
					for (int32 PixelX = 0; PixelX < 4; ++PixelX)
					{
						FLinearColor& Pixel = BlockPixels[PixelY * 4 + PixelX];

						if (bIgnoreTransparentPixels && Pixel.A < MinAlphaThreshold)
						{
							continue;
						}

						SampledPixels.Add(Pixel);
						SampledCount++;
					}
				}
			}
		}

		DMP_LOG(Log, TEXT("SampleTexturePixels - PF_DXT1: Total checked=%d, Sampled=%d"), TotalPixels, SampledCount);
	}
	else
	{
		DMP_LOG(Warning, TEXT("SampleTexturePixels - Unsupported pixel format: %d"), (int)PixelFormat);
	}

	// Unlock the data
	MipMap.BulkData.Unlock();

	DMP_LOG(Log, TEXT("SampleTexturePixels - Completed. Sampled pixels: %d"), SampledPixels.Num());
}

void UDreamAsyncAction_KMeansTexture::DecodeDXT1Block(const uint8* BlockData, FLinearColor* OutPixels)
{
	// DXT1 块由两个 16 位颜色和 6 个压缩像素组成（每个像素占 2 位）
	uint16 Color0 = *(uint16*)BlockData; // 第一个颜色
	uint16 Color1 = *(uint16*)(BlockData + 2); // 第二个颜色

	// 解包颜色数据（5 位红色，6 位绿色，5 位蓝色）
	FColor ColorA = UnpackDXT1Color(Color0);
	FColor ColorB = UnpackDXT1Color(Color1);

	// 生成 4 种颜色：Color0, Color1, 和两个插值颜色
	FColor Colors[4];
	Colors[0] = ColorA; // Color0
	Colors[1] = ColorB; // Color1
	Colors[2] = FColor(
		(ColorA.R + ColorB.R) / 2, // 红色分量的平均值
		(ColorA.G + ColorB.G) / 2, // 绿色分量的平均值
		(ColorA.B + ColorB.B) / 2, // 蓝色分量的平均值
		255 // Alpha 值不变
	);
	Colors[3] = FColor(
		(2 * ColorA.R + ColorB.R) / 3, // 红色分量的加权平均
		(2 * ColorA.G + ColorB.G) / 3, // 绿色分量的加权平均
		(2 * ColorA.B + ColorB.B) / 3, // 蓝色分量的加权平均
		255 // Alpha 值不变
	);

	// 解码压缩的像素数据
	uint32 ColorIndices = *(uint32*)(BlockData + 4); // 4 字节的像素数据，包含 16 个像素的索引

	// 对于每个像素，根据索引选择颜色
	for (int i = 0; i < 16; ++i)
	{
		uint8 Index = (ColorIndices >> (i * 2)) & 0x03; // 每个像素的索引占 2 位
		OutPixels[i] = FLinearColor::FromSRGBColor(Colors[Index]);
	}
}

FColor UDreamAsyncAction_KMeansTexture::UnpackDXT1Color(uint16 Color)
{
	// 解包 5-6-5 RGB 格式为 FColor
	uint8 R = (Color >> 11) & 0x1F;
	uint8 G = (Color >> 5) & 0x3F;
	uint8 B = Color & 0x1F;

	// 由于 DXT1 使用 5 位、6 位、5 位表示颜色，需要将这些值转换为 8 位
	return FColor(R << 3, G << 2, B << 3); // 5 -> 8, 6 -> 8 位的转换
}


void UDreamAsyncAction_KMeansTexture::InitializeClusters()
{
	ClusterCentroids.Empty();
	PixelClusterAssignments.SetNum(SampledPixels.Num());

	// Initialize centroids with random colors from sampled pixels
	for (int32 i = 0; i < NumClusters; ++i)
	{
		if (SampledPixels.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, SampledPixels.Num() - 1);
			ClusterCentroids.Add(SampledPixels[RandomIndex]);
		}
		else
		{
			// Fallback to random colors
			ClusterCentroids.Add(FLinearColor(
				FMath::FRand(),
				FMath::FRand(),
				FMath::FRand(),
				1.0f
			));
		}
	}

	// Initialize assignments to -1 (unassigned)
	for (int32& Assignment : PixelClusterAssignments)
	{
		Assignment = -1;
	}
}

bool UDreamAsyncAction_KMeansTexture::AssignPixelsToClusters()
{
	bool bAssignmentsChanged = false;

	for (int32 PixelIndex = 0; PixelIndex < SampledPixels.Num(); ++PixelIndex)
	{
		if (bIsCancelled)
		{
			return false;
		}

		const FLinearColor& Pixel = SampledPixels[PixelIndex];
		float MinDistance = FLT_MAX;
		int32 ClosestCluster = 0;

		// Find the closest cluster centroid
		for (int32 ClusterIndex = 0; ClusterIndex < ClusterCentroids.Num(); ++ClusterIndex)
		{
			float Distance = CalculateColorDistance(Pixel, ClusterCentroids[ClusterIndex]);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestCluster = ClusterIndex;
			}
		}

		// Update assignment if changed
		if (PixelClusterAssignments[PixelIndex] != ClosestCluster)
		{
			PixelClusterAssignments[PixelIndex] = ClosestCluster;
			bAssignmentsChanged = true;
		}
	}

	return bAssignmentsChanged;
}

void UDreamAsyncAction_KMeansTexture::UpdateClusterCentroids()
{
	TArray<FLinearColor> NewCentroids;
	TArray<int32> ClusterCounts;

	NewCentroids.SetNum(NumClusters);
	ClusterCounts.SetNum(NumClusters);

	// Initialize
	for (int32 i = 0; i < NumClusters; ++i)
	{
		NewCentroids[i] = FLinearColor::Black;
		ClusterCounts[i] = 0;
	}

	// Sum all pixels in each cluster
	for (int32 PixelIndex = 0; PixelIndex < SampledPixels.Num(); ++PixelIndex)
	{
		if (bIsCancelled)
		{
			return;
		}

		int32 ClusterIndex = PixelClusterAssignments[PixelIndex];
		if (ClusterIndex >= 0 && ClusterIndex < NumClusters)
		{
			NewCentroids[ClusterIndex] += SampledPixels[PixelIndex];
			ClusterCounts[ClusterIndex]++;
		}
	}

	// Calculate average (new centroid) for each cluster
	for (int32 ClusterIndex = 0; ClusterIndex < NumClusters; ++ClusterIndex)
	{
		if (ClusterCounts[ClusterIndex] > 0)
		{
			NewCentroids[ClusterIndex] /= ClusterCounts[ClusterIndex];
			ClusterCentroids[ClusterIndex] = NewCentroids[ClusterIndex];
		}
		// If cluster is empty, keep the old centroid
	}
}

float UDreamAsyncAction_KMeansTexture::CalculateColorDistance(const FLinearColor& Color1, const FLinearColor& Color2) const
{
	// Use weighted Euclidean distance in RGB space
	// Weight red/green/blue channels based on human perception
	float DR = (Color1.R - Color2.R) * 0.3f;
	float DG = (Color1.G - Color2.G) * 0.59f;
	float DB = (Color1.B - Color2.B) * 0.11f;

	return FMath::Sqrt(DR * DR + DG * DG + DB * DB);
}

TArray<FKMeansColorCluster> UDreamAsyncAction_KMeansTexture::GenerateResults()
{
	TArray<FKMeansColorCluster> Results;
	TArray<int32> ClusterCounts;
	ClusterCounts.SetNum(NumClusters);

	// Count pixels in each cluster
	for (int32 i = 0; i < NumClusters; ++i)
	{
		ClusterCounts[i] = 0;
	}

	for (int32 Assignment : PixelClusterAssignments)
	{
		if (Assignment >= 0 && Assignment < NumClusters)
		{
			ClusterCounts[Assignment]++;
		}
	}

	int32 TotalPixels = SampledPixels.Num();

	// Create result clusters, sorted by weight (most dominant first)
	for (int32 i = 0; i < NumClusters; ++i)
	{
		if (ClusterCounts[i] > 0)
		{
			float Weight = TotalPixels > 0 ? (float)ClusterCounts[i] / TotalPixels : 0.0f;
			Results.Add(FKMeansColorCluster(ClusterCentroids[i], Weight, ClusterCounts[i]));
		}
	}

	// Sort by weight (descending)
	Results.Sort([](const FKMeansColorCluster& A, const FKMeansColorCluster& B)
	{
		return A.Weight > B.Weight;
	});

	return Results;
}

void UDreamAsyncAction_KMeansTexture::CompleteTask(bool bSuccess)
{
	if (bIsCancelled)
	{
		bSuccess = false;
	}

	TArray<FKMeansColorCluster> Results;
	if (bSuccess)
	{
		Results = GenerateResults();
	}

	OnCompleted.Broadcast(Results, bSuccess);
	SetReadyToDestroy();
}
