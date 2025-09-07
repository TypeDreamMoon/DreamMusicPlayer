// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicPlayerExpansionData.h"
#include "DreamMusicPlayerExpansionData_Lyric.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Lyric")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansionData_Lyric : public UDreamMusicPlayerExpansionData
{
	GENERATED_BODY()

public:
	// 歌词解析文件类型
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite)
	EDreamMusicPlayerLyricParseFileType LyricParseFileType = EDreamMusicPlayerLyricParseFileType::LRC;

	// LRC歌词类型
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(EditConditionHides, EditCondition="LyricParseFileType == EDreamMusicPlayerLyricParseFileType::LRC"))
	EDreamMusicPlayerLrcLyricType LrcLyricType = EDreamMusicPlayerLrcLyricType::None;

	// 歌词行类型
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite)
	EDreamMusicPlayerLyricParseLineType LyricParseLineType = EDreamMusicPlayerLyricParseLineType::Romanization_Lyric;

	// 内容路径请在ProjectSetting -> DreamPlugins -> Musicplayer -> LyricContentPath 中配置
	UPROPERTY(Category="Lyric", EditAnywhere, BlueprintReadWrite, meta=(GetOptions = "DreamMusicPlayer.DreamMusicPlayerBlueprint.GetLyricFileNames"))
	FString LyricFileName;
};
