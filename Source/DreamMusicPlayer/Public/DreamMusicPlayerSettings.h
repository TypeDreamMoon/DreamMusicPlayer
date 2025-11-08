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
	static UDreamMusicPlayerSettings* Get();

public:
	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("DreamPlugin"); }
	/** The unique name for your section of settings, uses the class's FName. */
	virtual FName GetSectionName() const override { return TEXT("DreamMusicPlayerSetting"); }

public:
	// 歌词Content路径
	UPROPERTY(EditAnywhere, DisplayName="歌词Content路径", Category="Lyric", Config, meta=(LongPackageName))
	FDirectoryPath LyricContentPath;

	UPROPERTY(EditAnywhere, DisplayName="启用调试模式", Category="Debug", Config)
	bool bEnableDebugMode = false;

	UPROPERTY(EditAnywhere, DisplayName="启用拓展调试", Category="Debug", Config, meta=(EditConditionHides, EditCondition="bEnableDebugMode"))
	bool bEnableDebugExpansionMode = false;

	UPROPERTY(EditAnywhere, DisplayName="启用播放器Tick调试", Category="Debug", Config, meta=(EditConditionHides, EditCondition="bEnableDebugMode"))
	bool bEnableTickDebugMode = false;

	UPROPERTY(EditAnywhere, DisplayName="启用解析器调试模式", Category="Debug", Config, meta=(EditConditionHides, EditCondition="bEnableDebugMode"))
	bool bEnableParserDebugMode = false;

	UPROPERTY(VisibleDefaultsOnly, DisplayName="Version", Category="Version")
	FString Versions;
};
