#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamLyricAsset.generated.h"

/**
 * @brief 歌词资产统计信息
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricAssetStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalGroups = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalLines = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalWords = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FDreamMusicLyricTimestamp StartTime;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FDreamMusicLyricTimestamp EndTime;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	bool bHasWordTimings = false;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	bool bHasMultipleRoles = false;
};

/**
 * @brief 歌词资产类
 * 
 * 这是歌词文件在 UE 中的资产表示，包含完整的歌词数据、元数据和工具方法
 */
UCLASS(BlueprintType, meta = (DisplayName = "Dream Lyric Asset"))
class DREAMMUSICPLAYERLYRIC_API UDreamLyricAsset : public UObject
{
	GENERATED_BODY()

public:
	UDreamLyricAsset(const FObjectInitializer& ObjectInitializer);

	// ========== 核心数据 ==========
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Data", meta = (DisplayName = "Metadata"))
	FDreamMusicLyricMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Data", meta = (DisplayName = "Groups"))
	TArray<FDreamMusicLyricGroup> Groups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Info", meta = (DisplayName = "Source File Name"))
	FString SourceFileName;

	// ========== 查询方法 ==========

	/**
	 * @brief 根据时间查找对应的歌词组
	 * 
	 * @param Time 查询时间
	 * @param ToleranceSeconds 容差（秒）
	 * @return 找到的歌词组索引，如果未找到返回 INDEX_NONE
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Query")
	int32 FindGroupByTime(const FDreamMusicLyricTimestamp& Time, float ToleranceSeconds = 0.1f) const;

	/**
	 * @brief 根据时间获取对应的歌词组
	 * 
	 * @param Time 查询时间
	 * @param ToleranceSeconds 容差（秒）
	 * @return 找到的歌词组指针，如果未找到返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Query")
	FDreamMusicLyricGroup GetGroupByTime(const FDreamMusicLyricTimestamp& Time, float ToleranceSeconds = 0.1f);
	
	/**
	 * @brief 获取指定角色的所有行
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Query")
	TArray<FDreamMusicLyricLine> GetLinesByRole(EDreamMusicLyricTextRole Role) const;

	// ========== 统计信息 ==========

	/**
	 * @brief 获取资产统计信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	FLyricAssetStatistics GetStatistics() const;

	/**
	 * @brief 获取总时长（秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	float GetTotalDurationSeconds() const;

	/**
	 * @brief 获取总时长（时间跨度）
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	FDreamMusicLyricTimestamp GetTotalDuration() const;

	/**
	 * @brief 获取组数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	int32 GetGroupCount() const { return Groups.Num(); }

	/**
	 * @brief 获取总行数
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	int32 GetTotalLineCount() const;

	/**
	 * @brief 获取总单词数
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	int32 GetTotalWordCount() const;

	// ========== 工具方法 ==========

	/**
	 * @brief 对组按时间戳排序
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	void SortGroupsByTime();

	/**
	 * @brief 验证资产数据的有效性
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool Validate() const;

	/**
	 * @brief 获取验证错误列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	TArray<FString> GetValidationErrors() const;

	/**
	 * @brief 清空所有数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	void Clear();

	/**
	 * @brief 判断资产是否为空
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool IsEmpty() const;

	/**
	 * @brief 检查是否包含逐词时间信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool HasWordTimings() const;

	/**
	 * @brief 检查是否包含多种角色（原歌词、音译、翻译等）
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool HasMultipleRoles() const;

	// ============ 向后兼容接口 ============
	
	/**
	 * @brief 转换为旧的 FDreamMusicLyric 数组格式（向后兼容）
	 * @return TArray<FDreamMusicLyric> 旧格式的歌词数组
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric Asset|Compatibility")
	TArray<FDreamMusicLyric> ToLegacyLyrics() const;

	/**
	 * @brief 从旧的 FDreamMusicLyric 数组创建资产（向后兼容）
	 * @param LegacyLyrics 旧格式的歌词数组
	 * @param Outer 外部对象（用于创建资产）
	 * @return ULyricAsset* 创建的资产对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric Asset|Compatibility", meta = (CallInEditor = "true"))
	static UDreamLyricAsset* FromLegacyLyrics(const TArray<FDreamMusicLyric>& LegacyLyrics, UObject* Outer = nullptr);

	// ========== 元数据便捷访问 ==========

	/**
	 * @brief 获取标题
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetTitle() const { return Metadata.GetTitle(); }

	/**
	 * @brief 获取艺术家
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetArtist() const { return Metadata.GetArtist(); }

	/**
	 * @brief 获取专辑
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetAlbum() const { return Metadata.GetAlbum(); }

	/**
	 * @brief 获取创建者
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetCreator() const { return Metadata.GetCreator(); }

	
#if WITH_EDITOR
	// ========== 编辑器专用方法 ==========

	/**
	 * @brief 获取资产描述（用于内容浏览器）
	 */
	virtual FString GetDesc() override;
#endif
};
