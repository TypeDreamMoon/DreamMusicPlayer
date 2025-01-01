// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DreamAsyncAction_KMeansTexture.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FDreamColorKMeansCluster
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Centroid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLinearColor> Points;
};

UCLASS()
class DREAMMUSICPLAYER_API UDreamAsyncAction_KMeansTexture : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinish, const TArray<FLinearColor>&, ClusterColors);

	UPROPERTY(BlueprintAssignable)
	FOnFinish OnFinish;

public:
	UFUNCTION(BlueprintCallable,
		meta=(BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm =
			"WorldContextObject"))
	static UDreamAsyncAction_KMeansTexture* KMeansTexture(UTexture2D* Texture2D, int32 K, int32 MaxIterations,
	                                                      UObject* WorldContextObject);

private:
	TArray<FLinearColor> NodeColors;
	int32 NodeK;
	int32 NodeMaxIterations;
 
	UPROPERTY()
	UObject* WorldContext;
	UPROPERTY()
	UTexture2D* Texture;
 
	// 计算线性颜色之间的距离平方
	float LinearColorDistanceSquared(const FLinearColor& A, const FLinearColor& B);
	// 分配线性颜色到最近的聚类中心
	void AssignLinearColorToClusters(const TArray<FLinearColor>& Colors, TArray<FDreamColorKMeansCluster>& Clusters, TArray<int32>& ClusterIndices);
	// 更新聚类中心
	void UpdateLinearClusterCentroids(TArray<FDreamColorKMeansCluster>& Clusters);
	
	// 激活Node
	virtual void Activate() override;
};
