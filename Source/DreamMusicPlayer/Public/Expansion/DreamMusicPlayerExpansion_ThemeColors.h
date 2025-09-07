// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerExpansion_ThemeColors.generated.h"

class UDreamAsyncAction_KMeansTexture;
struct FKMeansColorCluster;
/**
 * 
 */
UCLASS(DisplayName = "Theme Colors")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_ThemeColors : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMusicPlayerThemeColorChanged, const TArray<FKMeansColorCluster>&, Colors, bool, bSuccess);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float SampleRate = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIgnoreTransparent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float AlphaThreshold = 0.5f;

	// Cover Theme Color Count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int CoverThemeColorCount = 4;

	// Max Iterations Count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int MaxIterationsCount = 3;

public:
	/**
	 * Extract theme colors from current music cover asynchronously
	 * @param ClusterCount Number of dominant colors to extract
	 * @param MaxIterations Maximum K-Means iterations
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions|Theme")
	void ExtractCoverThemeColors(int32 ClusterCount = 5, int32 MaxIterations = 100);

	/**
	 * Extract theme colors from texture
	 * @param Texture Texture to extract colors from
	 * @param ClusterCount Number of dominant colors to extract
	 * @param MaxIterations Maximum K-Means iterations
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions|Theme")
	void ExtractTextureThemeColors(UTexture2D* Texture, int32 ClusterCount, int32 MaxIterations);

protected:
	UFUNCTION()
	void OnThemeColorsExtracted(const TArray<FKMeansColorCluster>& ColorClusters, bool bSuccess);

	UPROPERTY()
	TObjectPtr<UDreamAsyncAction_KMeansTexture> CurrentKMeansTask;

public:
	UPROPERTY(BlueprintAssignable, Category = "Delegates|ThemeColor")
	FMusicPlayerThemeColorChanged OnThemeColorChanged;

protected:
	virtual void BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData) override;
	virtual void BP_Deinitialize_Implementation() override;
};
