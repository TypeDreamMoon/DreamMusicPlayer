// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DreamMusicPlayerSettings.generated.h"

/**
 * 
 */
UCLASS(DefaultConfig, Config=DreamMusicPlayer)
class DREAMMUSICPLAYER_API UDreamMusicPlayerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UDreamMusicPlayerSettings();
public:
	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("DreamPlugin"); }
	/** The unique name for your section of settings, uses the class's FName. */
	virtual FName GetSectionName() const override { return TEXT("DreamMusicPlayerSetting"); }
public:
	// KMeans Texture 算法映射材质
	UPROPERTY(EditAnywhere, DisplayName="K-均值算法映射材质", Category="KMeans Texture", Config)
	TSoftObjectPtr<UMaterialInterface> KMeansTextureMaterial;

	// KMeans Texture 算法材质槽位名称
	// 例如 "Texture"
	UPROPERTY(EditAnywhere, DisplayName="K-均值算法材质槽位名称", Category="KMeans Texture", Config)
	FName KMeansTextureSlotName = FName{TEXTVIEW("Texture")};

	// KMeans Texture 算法渲染目标大小
	UPROPERTY(EditAnywhere, DisplayName="K-均值算法渲染目标大小", Category="KMeans Texture", Config)
	FIntPoint KMeansTextureSize = FIntPoint(256, 256);

	// 歌词Content路径
	UPROPERTY(EditAnywhere, DisplayName="歌词Content路径", Category="Lyric", Config, meta=(LongPackageName))
	FDirectoryPath LyricContentPath;
};
