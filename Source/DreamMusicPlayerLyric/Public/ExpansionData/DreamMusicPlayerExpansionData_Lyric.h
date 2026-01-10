// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamLyricParserRuntime.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "UObject/SoftObjectPtr.h"
#include "DreamMusicPlayerExpansionData_Lyric.generated.h"

class UDreamLyricAsset;

/**
 * 
 */
UCLASS(DisplayName = "Lyric Expansion Data")
class DREAMMUSICPLAYERLYRIC_API UDreamMusicPlayerExpansionData_Lyric : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite)
	EDreamMuiscPlayerLyricSourceType LyricSourceType = EDreamMuiscPlayerLyricSourceType::File;
	// 歌词解析文件类型
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite)
	EDreamMusicPlayerLyricType LyricFileType = EDreamMusicPlayerLyricType::LRC;

	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(EditConditionHides, EditCondition="LyricSourceType == EDreamMuiscPlayerLyricSourceType::Asset"))
	TSoftObjectPtr<UDreamLyricAsset> LyricAsset;

	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(EditConditionHides, EditCondition="LyricSourceType == EDreamMuiscPlayerLyricSourceType::Stream"))
	FString LyricStreamSource;
	
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(EditConditionHides, EditCondition="LyricSourceType == EDreamMuiscPlayerLyricSourceType::File"))
	FDreamLyricParserOptions LyricParserOptions;

	/*
	// LRC歌词类型
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(EditConditionHides, EditCondition="LyricFileType == EDreamMusicPlayerLyricType::LRC"))
	EDreamMusicPlayerLrcLyricType LrcLyricType = EDreamMusicPlayerLrcLyricType::None;
	*/

	// 歌词行类型
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(EditConditionHides, EditCondition="LyricSourceType != EDreamMuiscPlayerLyricSourceType::Asset && LyricSourceType != EDreamMuiscPlayerLyricSourceType::Stream"))
	EDreamMusicPlayerLyricParseLineType LyricParseLineType = EDreamMusicPlayerLyricParseLineType::Romanization_Lyric;

	// 内容路径请在ProjectSetting -> DreamPlugins -> Musicplayer -> LyricContentPath 中配置
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(GetOptions = "DreamMusicPlayer.DreamMusicPlayerBlueprint.GetLyricFileNames", EditConditionHides, EditCondition="LyricSourceType != EDreamMuiscPlayerLyricSourceType::Asset && LyricSourceType != EDreamMuiscPlayerLyricSourceType::Stream"))
	FString LyricFileName;
};
