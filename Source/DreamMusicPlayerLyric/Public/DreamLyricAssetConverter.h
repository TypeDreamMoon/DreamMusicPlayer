#pragma once

#include "CoreMinimal.h"
#include "DreamLyricAsset.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamLyricAssetConverter.generated.h"

/**
 * @brief 歌词数据转换工具类
 * 
 * 用于在旧的 DreamMusicPlayer 歌词系统和新的 LyricAsset 系统之间进行转换
 * 保持向后兼容性
 */
UCLASS()
class DREAMMUSICPLAYERLYRIC_API ULyricAssetConverter : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief 将 FDreamMusicLyricTimestamp 转换为 FDreamMusicLyricTimestamp（现在类型已统一，直接返回）
	 */
	static FDreamMusicLyricTimestamp ConvertTimestamp(const FDreamMusicLyricTimestamp& OldTimestamp);

	/**
	 * @brief 将 FDreamMusicLyric 转换为 FDreamMusicLyricGroup
	 */
	static FDreamMusicLyricGroup ConvertLyric(const FDreamMusicLyric& OldLyric);

	/**
	 * @brief 将 FDreamMusicLyricGroup 转换为 FDreamMusicLyric
	 */
	static FDreamMusicLyric ConvertLyric(const FDreamMusicLyricGroup& NewGroup);

	/**
	 * @brief 将旧的歌词数组转换为 ULyricAsset
	 */
	static UDreamLyricAsset* ConvertLyricsToAsset(const TArray<FDreamMusicLyric>& OldLyrics, UObject* Outer = nullptr);

	/**
	 * @brief 将 ULyricAsset 转换为旧的歌词数组
	 */
	static TArray<FDreamMusicLyric> ConvertAssetToLyrics(const UDreamLyricAsset* Asset);
};

