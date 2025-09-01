// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "DreamAsyncAction_KMeansTexture.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FKMeansColorCluster
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FLinearColor Color;

	UPROPERTY(BlueprintReadOnly)
	float Weight; // Percentage of pixels in this cluster (0.0 to 1.0)

	UPROPERTY(BlueprintReadOnly)
	int32 PixelCount;

	FKMeansColorCluster()
		: Color(FLinearColor::White)
		, Weight(0.0f)
		, PixelCount(0)
	{
	}

	FKMeansColorCluster(FLinearColor InColor, float InWeight, int32 InPixelCount)
		: Color(InColor)
		, Weight(InWeight)
		, PixelCount(InPixelCount)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKMeansTextureCompleted, const TArray<FKMeansColorCluster>&, ColorClusters, bool, bSuccess);

UCLASS()
class DREAMMUSICPLAYER_API UDreamAsyncAction_KMeansTexture : public UCancellableAsyncAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FKMeansTextureCompleted OnCompleted;

    /**
     * Extract dominant colors from texture using K-Means clustering
     * @param Texture The texture to analyze
     * @param ClusterCount Number of color clusters to find (typically 3-8)
     * @param MaxIterations Maximum iterations for K-Means algorithm
     * @param SampleRate Sample rate for pixel sampling (1.0 = all pixels, 0.1 = every 10th pixel)
     * @param bIgnoreTransparent Whether to ignore transparent/semi-transparent pixels
     * @param AlphaThreshold Minimum alpha value to consider (0.0-1.0, used when bIgnoreTransparent is true)
     */
    UFUNCTION(BlueprintCallable, Category = "Dream Music Player", meta = (BlueprintInternalUseOnly = "true"))
    static UDreamAsyncAction_KMeansTexture* KMeansTextureAnalysis(
        UTexture2D* Texture,
        int32 ClusterCount = 5,
        int32 MaxIterations = 100,
        float SampleRate = 0.25f,
        bool bIgnoreTransparent = true,
        float AlphaThreshold = 0.5f
    );

    virtual void Activate() override;
    virtual void Cancel() override;

	bool bIsCancelled;
	inline int32 GetSamplePixelNum()
	{
		return SampledPixels.Num();
	}

private:
    // Input parameters
    UPROPERTY()
    TObjectPtr<UTexture2D> SourceTexture;
    
    int32 NumClusters;
    int32 MaxIterationsCount;
    float PixelSampleRate;
    bool bIgnoreTransparentPixels;
    float MinAlphaThreshold;

    // Processing data
    TArray<FLinearColor> SampledPixels;
    TArray<FLinearColor> ClusterCentroids;
    TArray<int32> PixelClusterAssignments;

    void ExecuteKMeans();
    void InitializeClusters();
    bool AssignPixelsToClusters();
    void UpdateClusterCentroids();
    float CalculateColorDistance(const FLinearColor& Color1, const FLinearColor& Color2) const;
    void SampleTexturePixels();
    TArray<FKMeansColorCluster> GenerateResults();
	void DecodeDXT1Block(const uint8* BlockData, FLinearColor* OutPixels);
	FColor UnpackDXT1Color(uint16 Color);
    void CompleteTask(bool bSuccess);
};
